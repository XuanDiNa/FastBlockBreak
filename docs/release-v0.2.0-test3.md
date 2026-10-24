# FastBlockBreak 0.2.0-test3

## English

This prerelease adds a persistent `config.json` Boolean `enabled` setting and a ModMenuBedrock external settings description. ModMenu Apply atomically replaces the JSON file; FastBlockBreak watches for file replacement and applies the new value without restarting Minecraft. The DLL remains loaded when disabled; the hook calls the original game function without applying the FastBlockBreak behavior.

The setting defaults to `true`. Missing `enabled` also means enabled. Invalid JSON, a non-Boolean value, duplicate `enabled` members, or a non-object root are rejected while preserving the last valid runtime state. Unknown JSON fields are retained by ModMenu Apply.

Automated parser, native policy, ModMenu schema and settings persistence checks have passed. The test3 DLL also passed read-only PE identity and 32-byte entry-guard validation against the exact `1.26.45.01`, `1.26.51.01`, and `1.26.52.03` executables. The user then confirmed the test3 package works in-game on all three versions, including the new settings flow.

| Game version | `.text` SHA-256 |
| --- | --- |
| `1.26.45.01` | `194006ACBAE5371133A09DC727E1176233D304A9E7A0F7AFC28D73E41D37F685` |
| `1.26.51.01` | `0CE1026651411CA94636B2B713F4B7D6838ADC3D652A85DDCCDF54E53DE0B4F5` |
| `1.26.52.03` | `6FA218D247874940235AACB8FD7E3BBF0E3D1E259630D7F83FCB536F06733AEB` |

The current 1.26.51.01 executable has a different full-file SHA-256 from the earlier audited copy, but its PE identity matches the second accepted profile, its `.text` hash matches the audited image, and both entry guards pass.

## 繁體中文

此預發佈版新增 `config.json` 的 `enabled` 布林開關，以及 ModMenuBedrock 外部設定描述檔。ModMenu 套用時會原子替換 JSON；FastBlockBreak 監看檔案替換並即時套用新值，不需重啟 Minecraft。停用時 DLL 仍保持載入，但 hook 會直接呼叫原始遊戲函式，不執行 FastBlockBreak 功能。

預設值為 `true`；缺少 `enabled` 也視為啟用。無效 JSON、非布林值、重複的 `enabled` 欄位或非物件根節點都會拒絕套用，保留上一個有效執行狀態。ModMenu 套用時會保留未公開的 JSON 欄位。

JSON parser、原生政策、ModMenu 描述檔 schema 與設定持久化自動檢查已通過。test3 DLL 也已對精確的 `1.26.45.01`、`1.26.51.01`、`1.26.52.03` 遊戲檔案通過唯讀 PE 身分與 32-byte 入口 guard 驗證；使用者隨後確認 test3 套件在三個版本的遊戲內均可正常使用，包含新的設定流程。

| 遊戲版本 | `.text` SHA-256 |
| --- | --- |
| `1.26.45.01` | `194006ACBAE5371133A09DC727E1176233D304A9E7A0F7AFC28D73E41D37F685` |
| `1.26.51.01` | `0CE1026651411CA94636B2B713F4B7D6838ADC3D652A85DDCCDF54E53DE0B4F5` |
| `1.26.52.03` | `6FA218D247874940235AACB8FD7E3BBF0E3D1E259630D7F83FCB536F06733AEB` |

目前的 1.26.51.01 執行檔完整檔案 SHA-256 與先前稽核副本不同，但 PE 身分符合第二組允許 profile，`.text` 雜湊與已稽核映像相同，兩個入口 guard 也都通過。
