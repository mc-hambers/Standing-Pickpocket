param([string]$DependencyRoot=$PSScriptRoot,[string]$ToolchainRoot)
if (!$ToolchainRoot) {$ToolchainRoot=Join-Path $DependencyRoot "tools/msvc/unpack"}
$ErrorActionPreference='Stop'
if (!(Test-Path "$ToolchainRoot/VC/Tools/MSVC")) {
    $vswhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
    $installed = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$installed) { throw 'MSVC C++ tools are missing. Install MSVC 19.51 x64 or supply -ToolchainRoot.' }
    $ToolchainRoot = $installed
    $installedSdk = "${env:ProgramFiles(x86)}/Windows Kits/10"
}
$vc=Get-ChildItem "$ToolchainRoot/VC/Tools/MSVC" -Directory | Sort-Object Name -Descending | Select-Object -First 1
$sdk=if($installedSdk){$installedSdk}else{"$ToolchainRoot/Windows Kits/10"}
$sv=Get-ChildItem "$sdk/Include" -Directory | Sort-Object Name -Descending | Select-Object -First 1
$env:INCLUDE="$($vc.FullName)/include;$sdk/Include/$($sv.Name)/ucrt;$sdk/Include/$($sv.Name)/shared;$sdk/Include/$($sv.Name)/um;$sdk/Include/$($sv.Name)/winrt"
$env:LIB="$($vc.FullName)/lib/x64;$sdk/Lib/$($sv.Name)/ucrt/x64;$sdk/Lib/$($sv.Name)/um/x64"
$cl="$($vc.FullName)/bin/Hostx64/x64/cl.exe"
$env:PATH="$(Split-Path $cl);$env:PATH"
$bundle="$DependencyRoot/vendor/prebuilt/commonlibsse-ng-prebuilt-v12.1.0-all-msvc-cmake"
New-Item -ItemType Directory -Force "$PSScriptRoot/build" | Out-Null
Push-Location "$PSScriptRoot/build"
try {
 $opts=@('/nologo','/utf-8','/MP4','/std:c++latest','/EHsc','/MD','/O2','/Gy','/Zc:preprocessor','/DWIN32_LEAN_AND_MEAN','/DNOMINMAX','/DENABLE_SKYRIM_SE=1','/DENABLE_SKYRIM_AE=1','/DENABLE_SKYRIM_VR=1','/DHAS_SKYRIM_MULTI_TARGETING=1','/DSPDLOG_FMT_EXTERNAL','/DSPDLOG_COMPILED_LIB',"/I$bundle/include","/I$bundle/extern/openvr/headers","/I$DependencyRoot/vendor/spdlog/include","/I$DependencyRoot/vendor/fmt/include")
 & $cl @opts /LD "$PSScriptRoot/src/Plugin.cpp" "$DependencyRoot/vendor/fmt/src/format.cc" "$DependencyRoot/vendor/spdlog/src/spdlog.cpp" "$DependencyRoot/vendor/spdlog/src/async.cpp" "$DependencyRoot/vendor/spdlog/src/cfg.cpp" "$DependencyRoot/vendor/spdlog/src/color_sinks.cpp" "$DependencyRoot/vendor/spdlog/src/file_sinks.cpp" "$DependencyRoot/vendor/spdlog/src/stdout_sinks.cpp" /link "$bundle/lib/CommonLibSSE.lib" "$bundle/lib/openvr_api.lib" bcrypt.lib d3d11.lib d3dcompiler.lib dbghelp.lib dxgi.lib version.lib kernel32.lib user32.lib advapi32.lib shell32.lib ole32.lib /OPT:REF /OPT:ICF "/IMPLIB:$PSScriptRoot/build/StandingPickpocket-v1.0.3.lib" "/OUT:$PSScriptRoot/Data/SKSE/Plugins/StandingPickpocket.dll" *> "$PSScriptRoot/logs/native-build.txt"
 if($LASTEXITCODE -ne 0){throw 'Native build failed; see logs/native-build.txt'}
 & $cl /nologo /EHsc /std:c++20 /MD "$PSScriptRoot/tests/ScopedStandingTests.cpp" "/Fe:$PSScriptRoot/build/stance-checks.exe" *> "$PSScriptRoot/logs/stance-checks.txt"
 if($LASTEXITCODE -ne 0){throw 'Stance test build failed'}
 & "$PSScriptRoot/build/stance-checks.exe" >> "$PSScriptRoot/logs/stance-checks.txt"
 if($LASTEXITCODE -ne 0){throw 'Stance restoration checks failed'}
} finally {Pop-Location}







