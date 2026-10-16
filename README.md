# FastBlockBreak

Minecraft Bedrock for Windows 原生客戶端插件：移除方塊間的客戶端破壞冷卻，並處理創造模式移動時的連續破壞。

A native client plugin for Minecraft Bedrock on Windows that removes the client-side delay between block breaks and handles continuous creative-mode breaking while moving.

[下載 / Downloads](https://github.com/XuanDiNa/FastBlockBreak/releases) · [回報問題 / Issues](https://github.com/XuanDiNa/FastBlockBreak/issues)

## 中文說明

### 功能

- 載入即啟用，無熱鍵、無設定檔。
- 僅作用於遊戲世界內的方塊破壞延續路徑；箱子、背包、選單與實體攻擊不在此掛鉤範圍。
- 清除方塊間的客戶端時間門檻；僅在確認為創造模式時，於原版呼叫期間暫時開啟移動距離門檻。
- 不更改方塊硬度、生存挖掘進度、工具速度、耐久、觸及距離、封包或伺服器規則。
- 同一個 DLL 自動辨識已核實版本；未知 PE 或任一入口檢查不符時自動停用，不修改遊戲 EXE。

這是獨立的基岩版實作，功能靈感來自 [Tweakeroo](https://github.com/maruohon/tweakeroo/) 的破壞冷卻選項，並非 Java 模組移植或官方產品。實際執行方式是掛鉤原生破壞延續函式，不另外每 tick 輪詢或強制執行兩次操作。

### 支援版本與測試範圍

目前版本：`0.2.0-test2`，僅支援 Windows x64。

| Minecraft Bedrock | 核實與測試狀態 |
|---|---|
| `1.26.45.01` | IDA 函式／欄位核實、離線 PE／入口檢查通過；使用者確認最新版站定、前後左右移動及跳躍連續破壞正常 |
| `1.26.51.01` | IDA 函式／欄位核實、離線 PE／入口檢查通過；尚未取得明確的遊戲實測確認 |
| `1.26.52.03` | 既有 `0.1.0` 已確認站定、各方向／斜向移動、跳躍、掉落及生存速度正常；`0.2.0-test2` 保留版本檢查並通過回歸測試，但不代表新版已重新完成全部遊戲實測 |

版本名稱相同不代表任意 EXE 都相容；只接受源碼中列出的原始／已核實載入器處理後 PE 身分及兩段完整 32-byte 入口檢查。離線測試不執行遊戲函式，也不等於多人、長時間穩定性或所有介面的完整驗收。伺服器仍可拒絕或限制破壞操作；使用前請確認伺服器規則。

### 安裝與移除

1. 從 Releases 下載 `FastBlockBreak_0.2.0-test2_MC26.45-26.51-26.52.zip`，不要下載 GitHub 自動產生的 Source code ZIP 作為插件。
2. 關閉目標遊戲版本。在 LeviLauncher 模組管理匯入 ZIP；或把 `FastBlockBreak.dll`、`manifest.json`、`mod.json` 放入該版本的 `mods/FastBlockBreak`。
3. 確認模組清單中的 FastBlockBreak 已啟用，再啟動遊戲。載入器須支援 `manifest.json` 的 `preload-native` 類型。
4. 先在測試世界確認創造模式站定、移動與跳躍時的連續破壞，再檢查生存挖掘速度、箱子／背包／選單、實體攻擊及離開重進世界。

移除時先關閉遊戲，再移除該版本的 `mods/FastBlockBreak` 資料夾。升級前請自行保留舊版本，不要在遊戲執行中替換 DLL。

### 本版修正

26.45 診斷發現世界使用「模式 5、子模式 1」，test1 漏判這條創造分支。test2 按原版方式解析世界預設模式，僅在它確實為創造時開啟移動門檻。臨時診斷日誌執行緒不編入此 DLL。

## English

### Features

- Enabled on load; no hotkey or configuration file.
- Removes the client-side delay between block breaks on the native world-only block-destruction continuation path.
- Supports continuous creative-mode breaking while moving by temporarily opening the verified movement-distance gate during the original call.
- Does not change survival break progress, block hardness, tool speed, durability, reach, packets, or server rules. Container/menu input and entity attacks are outside this hook.
- Automatically selects an exact verified version profile. Unsupported PE identities or mismatched function-entry guards keep the plugin inactive. The game executable is never modified by this plugin.

Inspired by the block-break cooldown option in [Tweakeroo](https://github.com/maruohon/tweakeroo/); this is an independent Bedrock implementation, not a Java mod port or an official Minecraft product. It hooks the native continuation function rather than polling or forcing two operations per tick.

### Supported versions and test scope

Release: `0.2.0-test2`. Windows x64 only.

| Minecraft Bedrock | Verification status |
|---|---|
| `1.26.45.01` | Native functions/layout reviewed in IDA; offline PE and entry guards pass. The user confirmed stationary, forward/back/left/right movement, and jumping continuous breaking with this version. |
| `1.26.51.01` | Native functions/layout reviewed in IDA; offline PE and entry guards pass. Explicit in-game confirmation is still pending. |
| `1.26.52.03` | Previous `0.1.0` gameplay confirmed stationary/directional/diagonal movement, jumping/falling, and vanilla survival speed. Profiles remain regression-tested in `0.2.0-test2`; this is not a claim that the complete gameplay matrix was repeated on the new DLL. |

Only the exact original/verified loader-processed PE identities and both full 32-byte entry guards encoded in the source are accepted. A matching version label alone is insufficient. Offline checks do not execute Minecraft functions or establish full multiplayer, long-session, or UI acceptance. Servers may reject or rate-limit actions independently; follow server rules.

### Installation and removal

1. Download `FastBlockBreak_0.2.0-test2_MC26.45-26.51-26.52.zip` from Releases, not GitHub's automatically generated Source code archive.
2. Close the target game version. Import the ZIP through LeviLauncher's mod manager, or place `FastBlockBreak.dll`, `manifest.json`, and `mod.json` in that version's `mods/FastBlockBreak` directory.
3. Enable FastBlockBreak in that version's mod list, then launch Minecraft. The loader must support the `preload-native` manifest type.
4. In a test world, check continuous creative breaking while stationary, moving, and jumping. Also check survival mining speed, containers/inventory/menus, entity attacks, and leaving/rejoining the world.

To uninstall, close Minecraft and remove that version's `mods/FastBlockBreak` directory. Keep your previous version before upgrading; never replace the DLL while Minecraft is running.

### Changes in this release

The 26.45 diagnostic identified mode `5` with world-default submode `1`, which test1 failed to classify as creative. Test2 follows the native world-default resolution and opens the movement gate only when creative is confirmed. The temporary diagnostic logging thread is not included.

## Build / 編譯

Requires Windows x64, Visual Studio 2022 C++ tools, and [xmake](https://xmake.io/). xmake fetches [MinHook 1.3.4](https://github.com/TsudaKageyu/minhook) from its package repository; there are no private dependency paths.

```powershell
pwsh -File tools/verify_release.ps1
```

This runs debug and release unit tests, builds the DLL, and validates its architecture and exports. Optional `-GameExecutables <paths>` additionally validates local supported game EXEs read-only. No Minecraft files are distributed. Runtime uses only `FastBlockBreak.cpp`, `VersionProfile.cpp`, and `NativeState.cpp`; policy/dispatch/bridge support files are retained for their regression tests, not linked into the plugin.

See [native profile notes](docs/native-profile.md) and [third-party notices](THIRD_PARTY_NOTICES.md). The downloadable DLL is the previously user-tested artifact; a local rebuild can have a different binary hash due to toolchain/dependency differences. Please report issues with your exact game version, game mode, movement, plugin version, and reproduction steps. / 回報問題時請附完整遊戲版本、模式、移動狀態、插件版本及重現步驟。
