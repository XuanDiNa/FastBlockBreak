# FastBlockBreak 0.2.0-test2

## 中文說明

### 功能

Windows x64 原生客戶端插件，移除方塊間的客戶端破壞冷卻，並支援創造模式移動時連續破壞。載入即啟用，無熱鍵、無設定檔。

只作用於遊戲世界內的原生方塊破壞延續路徑；箱子、背包、選單、實體攻擊不在此掛鉤範圍。不更改生存挖掘進度、方塊硬度、工具速度、耐久、觸及距離、封包或伺服器規則。不修改遊戲 EXE。

test2 修正 26.45 世界預設創造模式（模式 5、子模式 1）的辨識，解決 test1 移動／跳躍時仍受門檻限制的情況。臨時診斷日誌執行緒不包含在此 DLL 中。

### 支援版本與測試範圍

| Minecraft Bedrock（Windows x64） | 狀態 |
|---|---|
| `1.26.45.01` | 最新 test2 已由使用者確認站定、前後左右移動與跳躍連續破壞正常 |
| `1.26.51.01` | IDA 函式／欄位核實、離線 PE／入口檢查通過；遊戲實測確認仍待補充 |
| `1.26.52.03` | 既有 `0.1.0` 已確認站定、各方向／斜向移動、跳躍／掉落及生存速度正常；新版保留版本檢查並通過回歸測試，未重新完成全部遊戲實測 |

只接受核實過的 PE 身分與兩段完整 32-byte 入口檢查；未知版本自動停用。版本名稱相同不保證任意 EXE 相容。離線測試不代表多人、長時間穩定性或所有介面驗收，故維持預發佈狀態。伺服器仍可拒絕或限制破壞操作，請遵守伺服器規則。

### 安裝方法

1. 下載下方 `FastBlockBreak_0.2.0-test2_MC26.45-26.51-26.52.zip`，不是自動產生的 Source code ZIP。
2. 關閉目標遊戲版本，從 LeviLauncher 模組管理匯入 ZIP；或把 `FastBlockBreak.dll`、`manifest.json`、`mod.json` 放入該版本的 `mods/FastBlockBreak`。
3. 在模組清單啟用 FastBlockBreak，再啟動遊戲。載入器需要支援 `preload-native`。
4. 先在測試世界檢查站定、移動、斜向、跳躍／掉落連續破壞；另查生存速度、容器／選單／實體攻擊及離開重進世界。

移除時先關閉遊戲，再移除 `mods/FastBlockBreak`。升級前保留自己的舊版本，不要在遊戲執行中替換 DLL。ZIP 保留中文 `測試說明.txt`，並附英文說明。

## English

### Features

A native Windows x64 client plugin that removes the client-side delay between block breaks and supports continuous creative-mode breaking while moving. Enabled on load, with no hotkey or configuration file.

The hook applies only to the native in-world block-destruction continuation path. Container/inventory/menu input and entity attacks are outside this hook. It does not change survival break progress, hardness, tool speed, durability, reach, packets, or server rules, and never modifies the game executable.

Test2 fixes world-default creative classification (mode `5`, default `1`) on 26.45, addressing the movement/jumping gate left active in test1. The temporary diagnostic logging thread is not included.

### Supported versions and test scope

| Minecraft Bedrock (Windows x64) | Status |
|---|---|
| `1.26.45.01` | The user confirmed stationary, forward/back/left/right movement, and jumping continuous breaking with test2. |
| `1.26.51.01` | Native functions/layout reviewed in IDA; offline PE and function-entry checks pass. Explicit gameplay confirmation is pending. |
| `1.26.52.03` | Previous `0.1.0` gameplay confirmed stationary/directional/diagonal movement, jumping/falling and vanilla survival speed. Version guards and regression tests pass for the new version; the complete gameplay matrix was not repeated on test2. |

Only verified PE identities and both full 32-byte entry guards are accepted; other builds stay inactive. The same version label alone does not guarantee compatibility. Offline checks do not establish multiplayer, long-session or complete UI acceptance, so this remains a prerelease. Servers can independently reject or rate-limit actions; follow server rules.

### Installation

1. Download `FastBlockBreak_0.2.0-test2_MC26.45-26.51-26.52.zip` below, not the automatically generated Source code archive.
2. Close the target game version. Import the ZIP through LeviLauncher's mod manager, or place `FastBlockBreak.dll`, `manifest.json` and `mod.json` in that version's `mods/FastBlockBreak` directory.
3. Enable FastBlockBreak in the mod list and launch Minecraft. Your loader must support `preload-native` manifests.
4. Use a test world first. Check stationary/moving/diagonal/jumping/falling creative breaking, survival speed, containers/menus/entity attacks, and leaving/rejoining the world.

To uninstall, close Minecraft and remove `mods/FastBlockBreak`. Keep your previous version before upgrading; do not replace the DLL while Minecraft is running. The ZIP includes the original Chinese test instructions with updated status and an English section.

### Integrity / 完整性

The package preserves the previously user-tested DLL, not a newly substituted build. / 安裝包沿用使用者實測的 DLL，未替換為新編譯檔。

`FastBlockBreak.dll` SHA-256:

```text
60C6AD559B3106603FB2B8FFFA202B6C9DF65EF4CB8483347854837E170F2591
```

`SHA256SUMS.txt` contains the ZIP checksum. Source is available in this repository; no game binaries, IDA databases/decompiled code, runtime logs, or private local paths are distributed. / ZIP 校驗值見 `SHA256SUMS.txt`；倉庫提供源碼，不含遊戲檔案、IDA 資料庫／反編譯、執行日誌或私人路徑。

Please report issues with the exact game/plugin versions, game mode, movement state and reproduction steps. / 回報問題請附完整遊戲／插件版本、模式、移動狀態及重現步驟。
