#include "FastBlockBreak/Dispatch.h"
#include "FastBlockBreak/NativeState.h"
#include "FastBlockBreak/NativeBridge.h"
#include "FastBlockBreak/Policy.h"
#include "FastBlockBreak/VersionProfile.h"

#include <cstdlib>
#include <array>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

namespace {

int failures = 0;

void Check(bool condition, char const* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void TestPolicyRejectsEveryUnsafeState() {
    using namespace fastblockbreak;
    GateSnapshot good{true, true, true, false, true, HitType::Block};
    Check(IsEligibleBlockPass(good), "complete block gate must be eligible");
    Check(DesiredPassCount(good) == 2, "eligible block gate must request two passes");

    GateSnapshot cases[] = {
        {false, true, true, false, true, HitType::Block},
        {true, false, true, false, true, HitType::Block},
        {true, true, false, false, true, HitType::Block},
        {true, true, true, true, true, HitType::Block},
        {true, true, true, false, false, HitType::Block},
        {true, true, true, false, true, HitType::Unknown},
        {true, true, true, false, true, HitType::Miss},
        {true, true, true, false, true, HitType::Entity},
    };
    for (auto const& state : cases) {
        Check(!IsEligibleBlockPass(state), "one unsafe gate must reject eligibility");
        Check(DesiredPassCount(state) == 1, "unsafe state must retain one original pass");
    }
}

struct FakeDispatch {
    fastblockbreak::GateSnapshot snapshots[2]{};
    unsigned snapshotIndex{};
    unsigned clears{};
    unsigned passes{};
    bool passResult{true};
    bool cooldownClearedAfterTwoPasses{};
};

fastblockbreak::GateSnapshot Snapshot(void* context) noexcept {
    auto& fake = *static_cast<FakeDispatch*>(context);
    auto const index = fake.snapshotIndex < 2 ? fake.snapshotIndex++ : 1;
    return fake.snapshots[index];
}

void Clear(void* context) noexcept {
    auto& fake = *static_cast<FakeDispatch*>(context);
    ++fake.clears;
    fake.cooldownClearedAfterTwoPasses = fake.passes == 2;
}

bool Pass(void* context) noexcept {
    auto& fake = *static_cast<FakeDispatch*>(context);
    ++fake.passes;
    return fake.passResult;
}

void TestDispatchStopsWhenSecondSnapshotIsNotBlock() {
    using namespace fastblockbreak;
    FakeDispatch fake{{
        {true, true, true, false, true, HitType::Block},
        {true, true, true, false, true, HitType::Miss},
    }};
    auto result = DispatchAttackTick({&fake, Snapshot, Clear, Pass});
    Check(result.originalPasses == 1, "miss after first pass must suppress the second pass");
    Check(fake.clears == 0, "cooldown must remain untouched when the second pass is suppressed");
    Check(!result.secondPassCompleted, "suppressed second pass must not report completion");
}

void TestDispatchRunsExactlyTwoBlockPasses() {
    using namespace fastblockbreak;
    FakeDispatch fake{{
        {true, true, true, false, true, HitType::Block},
        {true, true, true, false, true, HitType::Block},
    }};
    auto result = DispatchAttackTick({&fake, Snapshot, Clear, Pass});
    Check(result.originalPasses == 2, "continuous block target must run exactly two passes");
    Check(fake.clears == 1, "only the added second pass may clear the click cooldown");
    Check(fake.cooldownClearedAfterTwoPasses,
          "the final original pass must not be able to restore the cooldown");
    Check(result.secondPassCompleted, "second block pass must report completion");
}

void TestDispatchFailsClosedForMissingCallbacks() {
    auto result = fastblockbreak::DispatchAttackTick({});
    Check(result.originalPasses == 0, "invalid callback set must perform no unsafe call");
}

void TestByteGuardHonorsMaskAndRejectsMalformedGuards() {
    using namespace fastblockbreak;
    std::uint8_t actual[4]{0x48, 0x8B, 0x05, 0xAA};
    ByteGuard guard{};
    guard.length = 4;
    guard.bytes[0] = 0x48;
    guard.bytes[1] = 0x8B;
    guard.bytes[2] = 0x05;
    guard.bytes[3] = 0x11;
    guard.mask[0] = guard.mask[1] = guard.mask[2] = 0xFF;
    Check(MatchBytes(actual, guard), "masked relocation byte must be ignored");
    guard.mask[3] = 0xFF;
    Check(!MatchBytes(actual, guard), "unmasked mismatch must reject the guard");
    guard.length = 0;
    Check(!MatchBytes(actual, guard), "zero-length guard must fail closed");
    guard.length = 33;
    Check(!MatchBytes(actual, guard), "oversized guard must fail closed");
}

void TestExecutableIdentityRejectsEveryFieldMismatch() {
    using namespace fastblockbreak;
    ExecutableIdentity expected{0x8664, 0x6AB54E37, 0x12C01000, 0x128CD801};
    Check(IdentityMatches(expected, expected), "exact executable identity must match");
    auto changed = expected;
    changed.machine = 0x14C;
    Check(!IdentityMatches(changed, expected), "machine mismatch must reject");
    changed = expected;
    ++changed.timeDateStamp;
    Check(!IdentityMatches(changed, expected), "timestamp mismatch must reject");
    changed = expected;
    ++changed.sizeOfImage;
    Check(!IdentityMatches(changed, expected), "image-size mismatch must reject");
    changed = expected;
    ++changed.checkSum;
    Check(!IdentityMatches(changed, expected), "checksum mismatch must reject");
}

void TestNativeStateMappingFailsClosed() {
    using namespace fastblockbreak;
    Check(MapNativeHitType(0) == HitType::Block, "native tile value must map to block");
    Check(MapNativeHitType(1) == HitType::Entity, "native entity value must map to entity");
    Check(MapNativeHitType(2) == HitType::Miss, "out-of-range entity must not become a block");
    Check(MapNativeHitType(3) == HitType::Miss, "native no-hit value must map to miss");
    Check(MapNativeHitType(4) == HitType::Unknown, "unknown native hit value must fail closed");
    Check(!IsNativeAttackHeld(0), "no action bits must not count as held attack");
    Check(IsNativeAttackHeld(0x2), "held action bit 0x2 must count as attack");
    Check(IsNativeAttackHeld(0x4), "held action bit 0x4 must count as attack");
}

void TestNativeSnapshotCombinesAllSafetyGates() {
    using namespace fastblockbreak;
    RawNativeSnapshot raw{true, true, true, true, false, true, 0};
    auto mapped = MapNativeSnapshot(raw);
    Check(IsEligibleBlockPass(mapped), "complete native gate must allow a block second pass");
    raw.foregroundProcess = false;
    Check(!MapNativeSnapshot(raw).focused, "foreground loss must reject focus");
    raw.foregroundProcess = true;
    raw.gameFocused = false;
    Check(!MapNativeSnapshot(raw).focused, "native focus loss must reject focus");
    raw.gameFocused = true;
    raw.rawHitType = 99;
    Check(MapNativeSnapshot(raw).hitType == HitType::Unknown,
          "unknown native hit discriminator must fail closed");
}

void TestGameModeDelayResetDoesNotChangeBreakProgress() {
    using namespace fastblockbreak;
    std::array<std::uint8_t, 0xA8> gameMode{};
    std::uint64_t delay = 0x1122334455667788ull;
    float previousProgress = 0.25f;
    float currentProgress = 0.75f;
    std::memcpy(gameMode.data() + 0x90, &delay, sizeof(delay));
    std::memcpy(gameMode.data() + 0x20, &previousProgress, sizeof(previousProgress));
    std::memcpy(gameMode.data() + 0x24, &currentProgress, sizeof(currentProgress));

    Check(ClearGameModeBlockBreakDelay(gameMode.data()),
          "a nonzero GameMode delay must report a real write");

    std::uint64_t actualDelay{};
    float actualPrevious{};
    float actualCurrent{};
    std::memcpy(&actualDelay, gameMode.data() + 0x90, sizeof(actualDelay));
    std::memcpy(&actualPrevious, gameMode.data() + 0x20, sizeof(actualPrevious));
    std::memcpy(&actualCurrent, gameMode.data() + 0x24, sizeof(actualCurrent));
    Check(actualDelay == 0, "GameMode no-block-break-until must be cleared");
    Check(actualPrevious == previousProgress, "previous break progress must remain unchanged");
    Check(actualCurrent == currentProgress, "current break progress must remain unchanged");
    Check(!ClearGameModeBlockBreakDelay(gameMode.data()),
          "an already-zero delay must avoid a redundant write");
    Check(!ClearGameModeBlockBreakDelay(nullptr),
          "a null GameMode must fail closed without a write");
}

std::int64_t __fastcall MovingContinueDestroyBlock(
    void* gameMode, void*, std::uint8_t, float*, std::uint8_t*) {
    auto delay = std::uint64_t{0x8877665544332211ull};
    std::memcpy(static_cast<std::uint8_t*>(gameMode) + 0x90, &delay, sizeof(delay));
    return 73;
}

void TestMovingTargetCannotRestoreDelayAfterOriginalCall() {
    using namespace fastblockbreak;
    std::array<std::uint8_t, 0xA8> gameMode{};
    auto initialDelay = std::uint64_t{0x1122334455667788ull};
    std::memcpy(gameMode.data() + 0x90, &initialDelay, sizeof(initialDelay));

    auto const result = InvokeContinueDestroyBlockWithoutDelay(
        MovingContinueDestroyBlock, gameMode.data(), nullptr, 0, nullptr, nullptr);

    std::uint64_t finalDelay{};
    std::memcpy(&finalDelay, gameMode.data() + 0x90, sizeof(finalDelay));
    Check(result == 73, "GameMode trampoline result must be preserved");
    Check(finalDelay == 0,
          "a moving-target branch must not restore noBlockBreakUntil on return");
}

std::int64_t __fastcall CreativeGameType(void*) { return 1; }
std::int64_t __fastcall SurvivalGameType(void*) { return 0; }
float observedMovementAccumulator = 0.0f;

std::int64_t __fastcall ObserveMovementGate(
    void* gameMode, void*, std::uint8_t, float*, std::uint8_t*) {
    std::memcpy(&observedMovementAccumulator,
                static_cast<std::uint8_t*>(gameMode) + 0x30,
                sizeof(observedMovementAccumulator));
    return 91;
}

void TestCreativeMovementGateIsBypassedWithoutAffectingSurvival() {
    using namespace fastblockbreak;
    std::array<std::uint8_t, 0xA8> gameMode{};
    std::array<std::uint8_t, 0x20> player{};
    void* playerPointer = player.data();
    std::memcpy(gameMode.data() + 0x08, &playerPointer, sizeof(playerPointer));

    observedMovementAccumulator = 0.0f;
    auto creativeResult = InvokeCreativeContinueDestroyBlockWithoutDelay(
        ObserveMovementGate, CreativeGameType, gameMode.data(), nullptr, 0, nullptr, nullptr);
    float finalAccumulator{};
    std::memcpy(&finalAccumulator, gameMode.data() + 0x30, sizeof(finalAccumulator));
    Check(creativeResult == 91, "creative trampoline result must be preserved");
    Check(observedMovementAccumulator > 1.0f,
          "creative movement gate must be open during the original call");
    Check(finalAccumulator == 0.0f,
          "temporary creative movement bypass must be cleared after return");

    observedMovementAccumulator = 0.0f;
    auto survivalResult = InvokeCreativeContinueDestroyBlockWithoutDelay(
        ObserveMovementGate, SurvivalGameType, gameMode.data(), nullptr, 0, nullptr, nullptr);
    Check(survivalResult == 91, "survival trampoline result must be preserved");
    Check(observedMovementAccumulator == 0.0f,
          "survival must not modify the creative movement accumulator");
}

int nativeTestMode = 5;
int nativeTestDefaultMode = 1;
unsigned nativeDefaultReads = 0;
unsigned nativeOriginalCalls = 0;
void* nativeExpectedPlayerContext = nullptr;
void* nativeExpectedReceiver = nullptr;

std::int64_t __fastcall ReadFixtureGameType(void* playerContext) {
    Check(playerContext == nativeExpectedPlayerContext,
          "game-type reader must receive player+8");
    return nativeTestMode;
}

std::int64_t __fastcall ReadFixtureDefaultGameType(void* receiver) {
    Check(receiver == nativeExpectedReceiver,
          "fallback getter must receive the original player-owned receiver");
    ++nativeDefaultReads;
    return nativeTestDefaultMode;
}

std::int64_t __fastcall ObserveInheritedMovementGate(
    void* gameMode, void*, std::uint8_t, float*, std::uint8_t*) {
    ++nativeOriginalCalls;
    auto* bytes = static_cast<std::uint8_t*>(gameMode);
    std::memcpy(&observedMovementAccumulator, bytes + 0x30, sizeof(float));
    auto const rewrittenDelay = std::uint64_t{444};
    std::memcpy(bytes + 0x90, &rewrittenDelay, sizeof(rewrittenDelay));
    return 91;
}

// Catches missing mode-5 resolution and accidental bypass of inherited survival.
void TestWorldDefaultCreativeMovementGateAndSurvivalExclusions() {
    using namespace fastblockbreak;
    struct Case { int mode; int defaultMode; bool open; unsigned defaultReads; };
    Case const cases[] = {
        {5, 1, true, 1}, {5, 0, false, 1}, {5, 2, false, 1}, {5, 3, false, 1},
        {1, 0, true, 0}, {0, 1, false, 0}, {2, 1, false, 0}, {-1, 1, false, 0},
    };
    alignas(8) std::array<std::uint8_t, 0xA8> gameMode{};
    alignas(8) std::array<std::uint8_t, 0x1E0> player{};
    std::array<std::uintptr_t, 0x468 / sizeof(std::uintptr_t)> vtable{};
    vtable[0x460 / sizeof(std::uintptr_t)] =
        reinterpret_cast<std::uintptr_t>(&ReadFixtureDefaultGameType);
    auto* vtablePointer = vtable.data();
    void* receiver = &vtablePointer;
    void* playerPointer = player.data();
    std::memcpy(gameMode.data() + 8, &playerPointer, sizeof(playerPointer));
    std::memcpy(player.data() + 0x1D8, &receiver, sizeof(receiver));
    nativeExpectedPlayerContext = player.data() + 8;
    nativeExpectedReceiver = receiver;
    for (auto const& test : cases) {
        nativeTestMode = test.mode;
        nativeTestDefaultMode = test.defaultMode;
        nativeDefaultReads = nativeOriginalCalls = 0;
        float const initialAccumulator = 0.25f;
        float const previousProgress = 0.4f, currentProgress = 0.7f;
        std::memcpy(gameMode.data() + 0x30, &initialAccumulator, sizeof(float));
        std::memcpy(gameMode.data() + 0x20, &previousProgress, sizeof(float));
        std::memcpy(gameMode.data() + 0x24, &currentProgress, sizeof(float));
        auto const result = InvokeCreativeContinueDestroyBlockWithoutDelay(
            ObserveInheritedMovementGate, ReadFixtureGameType,
            gameMode.data(), nullptr, 0, nullptr, nullptr);
        float finalAccumulator{}, finalPrevious{}, finalCurrent{};
        std::uint64_t finalDelay{};
        std::memcpy(&finalAccumulator, gameMode.data() + 0x30, sizeof(float));
        std::memcpy(&finalPrevious, gameMode.data() + 0x20, sizeof(float));
        std::memcpy(&finalCurrent, gameMode.data() + 0x24, sizeof(float));
        std::memcpy(&finalDelay, gameMode.data() + 0x90, sizeof(finalDelay));
        Check(result == 91 && nativeOriginalCalls == 1,
              "all native modes must preserve exactly one original call and its return");
        Check(nativeDefaultReads == test.defaultReads,
              "only mode 5 must resolve the inherited game type");
        Check(test.open ? observedMovementAccumulator > 1.0f
                        : observedMovementAccumulator == initialAccumulator,
              "mode5/submode1 must open the movement gate; noncreative modes must preserve it");
        Check(finalAccumulator == (test.open ? 0.0f : initialAccumulator),
              "only a creative bypass must clear the temporary movement accumulator");
        Check(finalPrevious == previousProgress && finalCurrent == currentProgress,
              "default-mode resolution must preserve both survival progress fields");
        Check(finalDelay == 0, "inherited mode must still clear delay rewritten by original");
    }

    nativeTestMode = 5;
    nativeTestDefaultMode = 1;
    auto const savedGetter = vtable[0x460 / sizeof(std::uintptr_t)];
    for (int invalid = 0; invalid < 3; ++invalid) {
        void* missingReceiver = invalid == 0 ? nullptr : receiver;
        vtablePointer = invalid == 1 ? nullptr : vtable.data();
        vtable[0x460 / sizeof(std::uintptr_t)] = invalid == 2 ? 0 : savedGetter;
        std::memcpy(player.data() + 0x1D8, &missingReceiver, sizeof(missingReceiver));
        float const accumulator = 0.25f;
        std::memcpy(gameMode.data() + 0x30, &accumulator, sizeof(float));
        nativeOriginalCalls = 0;
        auto const result = InvokeCreativeContinueDestroyBlockWithoutDelay(
            ObserveInheritedMovementGate, ReadFixtureGameType,
            gameMode.data(), nullptr, 0, nullptr, nullptr);
        Check(result == 91 && nativeOriginalCalls == 1 && observedMovementAccumulator == 0.25f,
              "missing fallback receiver/table/getter must keep the original call without bypass");
    }
}

void TestEveryVersionIdentityAndEntryGuard() {
    using namespace fastblockbreak;
    Check(SelectVersionProfile({}) == nullptr, "unknown PE identity must fail closed");
    Check(FindSupportedProfile(nullptr) == nullptr, "null host must fail closed");
    for (auto const* profile : SupportedProfiles()) {
        for (auto const& identity : profile->acceptedExecutables) {
            Check(SelectVersionProfile(identity) == profile,
                  "each original/installed identity must select its exact version");
            auto unknown = identity;
            ++unknown.checkSum;
            Check(SelectVersionProfile(unknown) == nullptr,
                  "unlisted checksum must not select a near-match profile");
        }
        auto const identity = profile->acceptedExecutables.front();
        auto* bytes = static_cast<std::uint8_t*>(VirtualAlloc(
            nullptr, identity.sizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
        Check(bytes != nullptr, "test image allocation must succeed");
        if (!bytes) { continue; }
        auto const module = reinterpret_cast<HMODULE>(bytes);
        auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(bytes);
        dos->e_magic = IMAGE_DOS_SIGNATURE;
        dos->e_lfanew = 0x80;
        auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(bytes + 0x80);
        nt->Signature = IMAGE_NT_SIGNATURE;
        nt->FileHeader.Machine = identity.machine;
        nt->FileHeader.TimeDateStamp = identity.timeDateStamp;
        nt->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
        nt->OptionalHeader.SizeOfImage = identity.sizeOfImage;
        nt->OptionalHeader.CheckSum = identity.checkSum;
        for (auto const* guard : {&profile->continueDestroyBlockHook, &profile->readGameType}) {
            std::memcpy(bytes + guard->rva, guard->bytes.data(), guard->length);
        }
        Check(FindSupportedProfile(module) == profile, "valid host must pass both entry guards");
        for (auto const* guard : {&profile->continueDestroyBlockHook, &profile->readGameType}) {
            for (std::size_t i = 0; i < guard->length; ++i) {
                bytes[guard->rva + i] ^= 1;
                Check(FindSupportedProfile(module) == nullptr,
                      "every entry byte mismatch must disable the plugin");
                bytes[guard->rva + i] ^= 1;
            }
        }
        for (auto const* other : SupportedProfiles()) {
            if (other != profile) {
                Check(!ValidateLoadedModule(module, *other), "cross-version profile must be rejected");
            }
        }
        auto malformed = *profile;
        malformed.readGameType.rva = identity.sizeOfImage - 16;
        Check(!ValidateLoadedModule(module, malformed), "entry extending past image end must reject");
        malformed.readGameType.length = 0;
        Check(!ValidateLoadedModule(module, malformed), "empty reader guard must reject");
        VirtualFree(bytes, 0, MEM_RELEASE);
    }
}

// Load bytes at their PE RVAs for validation only; never load imports or execute game code.
void TestOnDiskExecutable(char const* path) {
    using namespace fastblockbreak;
    std::ifstream stream(path, std::ios::binary);
    Check(stream.is_open(), "target executable must be readable");
    if (!stream) { return; }
    stream.seekg(0, std::ios::end);
    auto const size = stream.tellg();
    stream.seekg(0, std::ios::beg);
    if (size <= 0) { Check(false, "target executable must not be empty"); return; }
    std::vector<std::uint8_t> file(static_cast<std::size_t>(size));
    stream.read(reinterpret_cast<char*>(file.data()), size);
    Check(bool(stream), "complete executable read must succeed");
    if (!stream) { return; }
    Check(file.size() >= sizeof(IMAGE_DOS_HEADER), "target must contain a DOS header");
    if (file.size() < sizeof(IMAGE_DOS_HEADER)) { return; }
    auto const* dos = reinterpret_cast<IMAGE_DOS_HEADER const*>(file.data());
    if (dos->e_lfanew <= 0 || std::size_t(dos->e_lfanew) + sizeof(IMAGE_NT_HEADERS64) > file.size()) {
        Check(false, "target PE header must fit in file");
        return;
    }
    auto const* nt = reinterpret_cast<IMAGE_NT_HEADERS64 const*>(file.data() + dos->e_lfanew);
    auto* bytes = static_cast<std::uint8_t*>(VirtualAlloc(
        nullptr, nt->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    Check(bytes != nullptr, "disk validation image must allocate");
    if (!bytes) { return; }
    if (nt->OptionalHeader.SizeOfHeaders <= file.size() &&
        nt->OptionalHeader.SizeOfHeaders <= nt->OptionalHeader.SizeOfImage) {
        std::memcpy(bytes, file.data(), nt->OptionalHeader.SizeOfHeaders);
        ExecutableIdentity identity{};
        auto module = reinterpret_cast<HMODULE>(bytes);
        Check(ReadExecutableIdentity(module, identity), "on-disk PE identity must decode");
        auto const* profile = SelectVersionProfile(identity);
        Check(profile != nullptr, "on-disk game version must be supported");
        if (profile) {
            auto const* sections = IMAGE_FIRST_SECTION(nt);
            for (auto const* guard : {&profile->continueDestroyBlockHook, &profile->readGameType}) {
                bool copied = false;
                for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
                    auto const& section = sections[i];
                    if (guard->rva >= section.VirtualAddress &&
                        guard->rva + guard->length <=
                            std::uint64_t(section.VirtualAddress) + section.SizeOfRawData) {
                        auto const raw = section.PointerToRawData + guard->rva - section.VirtualAddress;
                        if (raw + guard->length <= file.size()) {
                            std::memcpy(bytes + guard->rva, file.data() + raw, guard->length);
                            copied = true;
                        }
                    }
                }
                Check(copied, "on-disk native entry must exist at the verified RVA");
            }
            Check(FindSupportedProfile(module) == profile, "real PE must pass version and entry validation");
            std::cout << "Validated " << profile->gameVersion << ": " << path << '\n';
        }
    } else {
        Check(false, "target headers must fit in image");
    }
    VirtualFree(bytes, 0, MEM_RELEASE);
}

} // namespace

int main(int argc, char** argv) {
    TestPolicyRejectsEveryUnsafeState();
    TestDispatchStopsWhenSecondSnapshotIsNotBlock();
    TestDispatchRunsExactlyTwoBlockPasses();
    TestDispatchFailsClosedForMissingCallbacks();
    TestByteGuardHonorsMaskAndRejectsMalformedGuards();
    TestExecutableIdentityRejectsEveryFieldMismatch();
    TestNativeStateMappingFailsClosed();
    TestNativeSnapshotCombinesAllSafetyGates();
    TestGameModeDelayResetDoesNotChangeBreakProgress();
    TestMovingTargetCannotRestoreDelayAfterOriginalCall();
    TestCreativeMovementGateIsBypassedWithoutAffectingSurvival();
    TestWorldDefaultCreativeMovementGateAndSurvivalExclusions();
    TestEveryVersionIdentityAndEntryGuard();
    for (int i = 1; i < argc; ++i) {
        TestOnDiskExecutable(argv[i]);
    }
    if (failures != 0) {
        std::cerr << failures << " test assertion(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "FastBlockBreak policy/dispatch tests passed\n";
    return EXIT_SUCCESS;
}
