# FastBlockBreak

[English](#english) · [繁體中文](#繁體中文)

## English

Smoother continuous block breaking in Minecraft Bedrock. Removes the client-side delay between block breaks, including while moving in creative mode.

A client-side native DLL loaded through LeviLauncher. No behavior pack or server plugin is required.

[Releases](https://github.com/XuanDiNa/FastBlockBreak/releases) · [Report an issue](https://github.com/XuanDiNa/FastBlockBreak/issues)

### Features

- **Continuous breaking:** removes the client-side delay between block breaks.
- **Creative movement:** supports consecutive breaking while moving or jumping.
- **Vanilla mining:** leaves survival mining speed, tool durability, and reach unchanged. Container/menu controls and entity attacks are outside the hook.
- **Ready on load:** enabled by default, with no hotkey required.
- **JSON switch:** set `config.json`'s `enabled` Boolean to enable or disable the feature.
- **ModMenu hot reload:** the bundled `modmenu.settings.json` exposes the same switch and applies it without restarting Minecraft; disabling bypasses the hook without unloading the DLL.

### Requirements

- Windows x64
- Minecraft **1.26.45.01 / 1.26.51.01 / 1.26.52.03**
- LeviLauncher with native DLL plugin support (`preload-native`)

One DLL supports these three exact game versions. Different binaries with the same version label may not be compatible; the plugin stays inactive if its version checks fail.

### Installation

1. Fully close Minecraft. If upgrading, back up the old plugin **outside the mods directory**.
2. Download the plugin ZIP from [Releases](https://github.com/XuanDiNa/FastBlockBreak/releases). The automatic source-code archives are not installable packages.
3. Import the ZIP through LeviLauncher's mod manager, or extract the package files into the selected game version's `mods/FastBlockBreak` folder. Keep `config.json` and `modmenu.settings.json` beside the DLL and metadata.
4. Enable FastBlockBreak and launch Minecraft. Try it in a test world first.

Do not replace DLLs while Minecraft is running or load old and new copies together. To uninstall, close the game and remove the plugin folder. To roll back, restore the old DLL and metadata from your backup. This plugin cannot be imported as a `.mcpack`.

### Notes and feedback

**0.2.0-test3 is a prerelease.** This build adds the JSON/ModMenu switch. Its native profiles pass read-only PE and entry-guard checks on all three exact supported executables, and the user confirmed gameplay on all three versions:

- **1.26.45.01:** gameplay and the test3 package confirmed working.
- **1.26.51.01:** gameplay and the test3 package confirmed working.
- **1.26.52.03:** gameplay and the test3 package confirmed working.

Multiplayer and long-session stability still need broader testing. The plugin does not modify the game executable or construct packets. Server-side restrictions still apply; follow server rules.

Feedback and issue reports are welcome! Include your exact game version, plugin version, game mode, movement state, and steps to reproduce.

Technical details: [release notes and test scope](docs/release-v0.2.0-test3.md) · [native compatibility](docs/native-profile.md). Build and verification tools: [xmake](xmake.lua) · [verifier](tools/verify_release.ps1).

Inspired by [Tweakeroo](https://github.com/maruohon/tweakeroo/)'s block-break cooldown option. This is an independent Bedrock implementation, not an official port or an official Minecraft product. See [third-party notices](THIRD_PARTY_NOTICES.md).

## 繁體中文

讓 Minecraft 基岩版的連續破壞更順暢。移除方塊間的客戶端破壞冷卻，支援創造模式移動時連續破壞。

透過 LeviLauncher 載入的客戶端原生 DLL 插件，不需要行為包或伺服器插件。

[下載 0.2.0-test2](https://github.com/XuanDiNa/FastBlockBreak/releases/tag/v0.2.0-test2) · [回報問題](https://github.com/XuanDiNa/FastBlockBreak/issues)

### 主要功能

- **連續破壞**：移除方塊間的客戶端破壞冷卻。
- **創造模式移動**：移動或跳躍時，也能連續破壞方塊。
- **保留原版挖掘**：不改變生存挖掘速度、工具耐久或觸及距離；容器、選單操作與實體攻擊不在掛鉤範圍。
- **載入即啟用**：預設啟用，不需要熱鍵。
- **JSON 開關**：將 `config.json` 的 `enabled` 布林值設為 `true` 或 `false`，即可啟用或停用功能。
- **ModMenu 即時調整**：隨附的 `modmenu.settings.json` 提供相同開關，不需重啟 Minecraft；停用只繞過功能，不會卸載 DLL。

### 環境需求

- Windows x64
- Minecraft **1.26.45.01／1.26.51.01／1.26.52.03**
- 支援原生 DLL 插件的 LeviLauncher（`preload-native`）

同一個 DLL 支援上述三個精確版本。相同版本名稱的不同遊戲檔案不一定相容；版本校驗不符時，插件不會啟用。

### 安裝方式

1. 完整退出 Minecraft。升級前，先將舊插件備份到 **mods 目錄外面**。
2. 從 [Releases](https://github.com/XuanDiNa/FastBlockBreak/releases) 下載插件 ZIP，不要下載自動產生的 Source code 壓縮檔。
3. 在 LeviLauncher 模組管理中匯入 ZIP；或將套件檔案解壓縮到對應遊戲版本的 `mods/FastBlockBreak` 資料夾。`config.json` 與 `modmenu.settings.json` 需和 DLL、版本描述檔放在一起。
4. 啟用 FastBlockBreak，再啟動遊戲。建議先在測試世界確認效果。

不要在遊戲執行中替換 DLL，也不要同時載入新舊插件。移除時，先退出遊戲再刪除插件資料夾；需要回退時，還原備份的舊 DLL 與版本描述檔。本插件不能當作 `.mcpack` 匯入。

### 注意事項與回饋

**0.2.0-test3 仍是測試版。** 本版本新增 JSON／ModMenu 開關；三個精確支援遊戲檔案均通過唯讀 PE 與入口 guard 檢查，且使用者已確認三個版本均可正常使用：

- **1.26.45.01**：test3 已確認遊戲內正常使用。
- **1.26.51.01**：test3 已確認遊戲內正常使用。
- **1.26.52.03**：test3 已確認遊戲內正常使用。

多人與長時間穩定性仍需要更完整的測試。插件不修改遊戲 EXE，也不自行組封包；伺服器仍可限制破壞操作，使用前請確認伺服器規則。

歡迎回報問題或提出建議！請附上精確遊戲版本、插件版本、遊戲模式、移動狀態及重現步驟。

技術文件：[版本記錄與測試範圍](docs/release-v0.2.0-test3.md) · [原生相容性](docs/native-profile.md)。建置與驗證工具：[xmake](xmake.lua) · [驗證腳本](tools/verify_release.ps1)。

本專案參考 [Tweakeroo](https://github.com/maruohon/tweakeroo/) 的破壞冷卻功能，是獨立的基岩版實作，並非官方移植或官方 Minecraft 產品。第三方授權見[相關聲明](THIRD_PARTY_NOTICES.md)。
