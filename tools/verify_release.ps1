param([string[]]$GameExecutables = @())
$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
Push-Location $project
try {
    foreach ($mode in 'debug','release') {
        & xmake f -y -m $mode
        if ($LASTEXITCODE) { throw "$mode configure failed" }
        & xmake -y -r FastBlockBreakTests
        if ($LASTEXITCODE) { throw "$mode test build failed" }
        & xmake run FastBlockBreakTests @GameExecutables
        if ($LASTEXITCODE) { throw "$mode tests failed" }
    }
    & xmake -y -r FastBlockBreak
    if ($LASTEXITCODE) { throw 'release build failed' }
    $dll = Join-Path $project 'build/windows/x64/release/FastBlockBreak.dll'
    $bytes = [IO.File]::ReadAllBytes($dll)
    $pe = [BitConverter]::ToInt32($bytes, 0x3C)
    if ([BitConverter]::ToUInt16($bytes, $pe + 4) -ne 0x8664) { throw 'DLL is not AMD64' }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    $dumpbin = @(& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'VC/Tools/MSVC/**/bin/Hostx64/x64/dumpbin.exe') | Select-Object -First 1
    if (!$dumpbin) { throw 'dumpbin.exe not found' }
    $exports = (& $dumpbin /exports $dll | Out-String)
    if ($LASTEXITCODE) { throw 'export inspection failed' }
    foreach ($name in 'FastBlockBreakInit','FastBlockBreakStop','FastBlockBreakStatus','FastBlockBreakEnabled') {
        if ($exports -notmatch "\b$([regex]::Escape($name))\b") { throw "missing export: $name" }
    }
    $manifest = Get-Content -Raw -Encoding UTF8 manifest.json | ConvertFrom-Json
    $metadata = Get-Content -Raw -Encoding UTF8 mod.json | ConvertFrom-Json
    if ($manifest.version -ne $metadata.version -or $manifest.entry -ne 'FastBlockBreak.dll' -or $manifest.type -ne 'preload-native') { throw 'metadata mismatch' }
    $config = Get-Content -Raw -Encoding UTF8 config.json | ConvertFrom-Json
    if ($config.enabled -isnot [bool] -or !$config.enabled) { throw 'default config must enable the feature' }
    $description = Get-Content -Raw -Encoding UTF8 modmenu.settings.json | ConvertFrom-Json
    if ($description.plugin.id -ne $manifest.name -or $description.plugin.entry -ne $manifest.entry -or $manifest.version -notin $description.plugin.versions -or $description.config.file -ne 'config.json' -or $description.config.reload -ne 'hot' -or $description.fields.Count -ne 1 -or $description.fields[0].path -ne '/enabled' -or $description.fields[0].type -ne 'boolean') { throw 'ModMenu description mismatch' }
    Get-FileHash -LiteralPath $dll -Algorithm SHA256
    Write-Host 'Debug/release tests, release build, AMD64, exports and metadata checks passed. Gameplay is not tested by this script.'
} finally { Pop-Location }
