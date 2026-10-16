# Native profile contract

The addresses/layout below were reviewed separately for each supported binary in IDA 9.3. They are not portable guesses or a promise of support for other builds. No game executable, database, decompiled function, or disassembly listing is included.

| Game version | continueDestroyBlock RVA | ReadGameType RVA |
|---|---|---|
| 1.26.45.01 | 0x2D2E310 | 0x380C960 |
| 1.26.51.01 | 0x259B230 | 0x253DC40 |
| 1.26.52.03 | 0x259AFD0 | 0x253D9E0 |

`VersionProfile.cpp` contains the accepted PE machine, timestamp, image size and checksum for each original/verified loader-processed build. Both full 32-byte function entries must match before hook installation. Unknown profiles fail closed.

The runtime clears the uint64 timer at `GameMode +0x90` before and after **one** original continuation call. For creative only, it temporarily sets the float accumulator at `+0x30` to `1.0001` and clears it afterward. Progress fields `+0x20/+0x24` remain untouched.

The player pointer is `[GameMode+0x08]`; the game-type reader receives `player+0x08`. Creative is type `1`, or type `5` when the world-default getter returns `1`. The latter receiver is `[player+0x1D8]`, using virtual slot `+0x460`. These contracts were checked in all three builds.

Regression tests cover timer ordering, creative/world-default creative movement, inherited survival, malformed/missing receivers, version identities, entry guards, and synthetic mapped images. Optional on-disk validation maps PE headers and guarded entries read-only and does not execute game code.
