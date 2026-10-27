param([string]$BuildDependencies = (Join-Path (Split-Path $PSScriptRoot -Parent) "../Pickpocketing-Loot"))
$ErrorActionPreference = 'Stop'
$projectDir = Split-Path $PSScriptRoot -Parent
$BuildDependencies = (Resolve-Path $BuildDependencies).Path
$toolDir = Join-Path $BuildDependencies 'tools'
$compilerDir = (Get-ChildItem (Join-Path $toolDir 'msvc/VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
$sdkDir = Join-Path $toolDir 'msvc/Windows Kits/10'
$sdkVersion = (Get-ChildItem (Join-Path $sdkDir 'Include') -Directory | Sort-Object Name -Descending | Select-Object -First 1).Name
$cmakeDir = Join-Path $toolDir 'python-packages/cmake/data/bin'
$ninjaDir = Join-Path $toolDir 'ninja'
$env:PATH = "$compilerDir/bin/Hostx64/x64;$sdkDir/bin/$sdkVersion/x64;$cmakeDir;$ninjaDir;$env:PATH"
$env:INCLUDE = "$compilerDir/include;$sdkDir/Include/$sdkVersion/ucrt;$sdkDir/Include/$sdkVersion/shared;$sdkDir/Include/$sdkVersion/um;$sdkDir/Include/$sdkVersion/winrt;$sdkDir/Include/$sdkVersion/cppwinrt"
$env:LIB = "$compilerDir/lib/x64;$sdkDir/Lib/$sdkVersion/ucrt/x64;$sdkDir/Lib/$sdkVersion/um/x64"
$env:VCPKG_FORCE_SYSTEM_BINARIES = '1'
$env:VCPKG_ROOT = Join-Path $toolDir 'vcpkg'
$env:VCPKG_DOWNLOADS = Join-Path $toolDir 'vcpkg-downloads'
$env:VCPKG_DEFAULT_BINARY_CACHE = Join-Path $toolDir 'vcpkg-cache'
New-Item -ItemType Directory -Force $env:VCPKG_DEFAULT_BINARY_CACHE | Out-Null
$commonSource = Join-Path $projectDir 'vendor/CommonLibSSE-NG'
$commonLibrary = ''
if (Test-Path "$projectDir/../Pickpocket-Overhaul/build/vendor/CommonLibSSE-NG/CommonLibSSE.lib") {
    $commonSource = "$projectDir/../Pickpocket-Overhaul/vendor/CommonLibSSE-NG"
    $commonLibrary = "$projectDir/../Pickpocket-Overhaul/build/vendor/CommonLibSSE-NG/CommonLibSSE.lib"
}
& "$cmakeDir/cmake.exe" -S $projectDir -B "$projectDir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION=Release "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_OVERLAY_TRIPLETS=$BuildDependencies/cmake/triplets" "-DVCPKG_INSTALLED_DIR=$BuildDependencies/vcpkg_installed" -DVCPKG_TARGET_TRIPLET=x64-portable -DVCPKG_HOST_TRIPLET=x64-portable -DVCPKG_MANIFEST_INSTALL=OFF "-DCOMMONLIB_SOURCE_DIR=$commonSource" "-DCOMMONLIB_PREBUILT_LIBRARY=$commonLibrary"
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }
& "$cmakeDir/cmake.exe" --build "$projectDir/build" --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
& "$cmakeDir/ctest.exe" --test-dir "$projectDir/build" --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
