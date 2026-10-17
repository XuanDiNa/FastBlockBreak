# FastBlockBreak

讓 Minecraft 基岩版的連續破壞更順暢。移除方塊間的客戶端破壞冷卻，支援創造模式移動時連續破壞。

Smoother continuous block breaking in Minecraft Bedrock. Removes the client-side delay between block breaks, including while moving in creative mode.

[下載 / Download](https://github.com/XuanDiNa/FastBlockBreak/releases) · [回報問題 / Report an issue](https://github.com/XuanDiNa/FastBlockBreak/issues)

## 功能

- 載入即啟用，無需熱鍵或設定。
- 支援創造模式站定、移動與跳躍時的連續破壞。
- 不改變生存挖掘速度、工具耐久或觸及距離，不影響容器與選單操作。
- 自動辨識支援的版本；遇到不相容的遊戲檔案時停用，不修改遊戲 EXE。

## 支援版本 / Compatibility

目前版本 / Current release：`0.2.0-test2` · **Windows x64**

| Minecraft Bedrock | 測試狀態 / Test status |
|---|---|
| `1.26.45.01` | 最新版已實測站定、前後左右移動及跳躍。 / Current release tested while stationary, moving in all four directions, and jumping. |
| `1.26.51.01` | 已通過離線檢查，遊戲實測待確認。 / Offline checks passed; in-game confirmation pending. |
| `1.26.52.03` | `0.1.0` 已實測；最新版通過回歸測試，尚未重做完整遊戲測試。 / Gameplay tested on `0.1.0`; current release passes regression tests but has not had a full gameplay retest. |

目前仍為預發佈版。相同版本名稱的不同遊戲檔案不一定相容；多人與長時間穩定性尚未完整驗證。伺服器仍可限制破壞操作，使用前請確認伺服器規則。

This is a prerelease. Different game binaries with the same version label may not be compatible. Multiplayer and long-session stability have not been fully verified, and server-side restrictions still apply.

## 安裝

1. 從 [Releases](https://github.com/XuanDiNa/FastBlockBreak/releases) 下載插件 ZIP，**不是 Source code**。
2. 關閉遊戲，在 LeviLauncher 模組管理中匯入 ZIP。
3. 啟用 FastBlockBreak，再啟動遊戲。建議先在測試世界確認效果。

也可將 `FastBlockBreak.dll`、`manifest.json`、`mod.json` 放入對應版本的 `mods/FastBlockBreak` 資料夾。載入器須支援 `preload-native`。移除插件時，先關閉遊戲，再移除該資料夾；升級前請保留舊版。

## Features

- Enabled on load, with no hotkey or configuration needed.
- Continuous creative-mode breaking while stationary, moving, or jumping.
- Leaves survival mining speed, tool durability, reach, and container/menu controls unchanged.
- Automatically selects a supported version and stays inactive on incompatible binaries. Does not modify the game executable.

## Installation

1. Download the plugin ZIP from [Releases](https://github.com/XuanDiNa/FastBlockBreak/releases), **not Source code**.
2. Close Minecraft and import the ZIP through LeviLauncher's mod manager.
3. Enable FastBlockBreak and launch the game. Try it in a test world first.

For manual installation, place `FastBlockBreak.dll`, `manifest.json`, and `mod.json` in the target version's `mods/FastBlockBreak` folder. Your loader must support `preload-native`. To uninstall, close Minecraft and remove that folder. Keep your previous version before upgrading.

## 開發 / Development

需要 / Requires：Windows x64、Visual Studio 2022 C++ tools、[xmake](https://xmake.io/)。MinHook 1.3.4 由 xmake 自動取得。 / xmake fetches MinHook 1.3.4 automatically.

```powershell
pwsh -File tools/verify_release.ps1
```

執行單元測試、編譯並檢查 DLL；不包含遊戲實測。 / Runs unit tests, builds the DLL, and checks its architecture and exports; does not test gameplay.

[版本與修正記錄 / Release notes](docs/release-v0.2.0-test2.md) · [技術說明 / Native profiles](docs/native-profile.md) · [第三方聲明 / Third-party notices](THIRD_PARTY_NOTICES.md)

靈感來自 [Tweakeroo](https://github.com/maruohon/tweakeroo/) 的破壞冷卻功能，為獨立的基岩版實作，非官方 Minecraft 產品。歡迎回報問題，請附上遊戲版本、插件版本及重現步驟。

Inspired by Tweakeroo's block-break cooldown option. An independent Bedrock implementation, not an official Minecraft product. Feedback is welcome—please include your game version, plugin version, and steps to reproduce.
