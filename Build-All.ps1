$ErrorActionPreference='Stop'
& "$PSScriptRoot/Build-Native.ps1"
$imports="$PSScriptRoot/Data/Scripts/Source;$PSScriptRoot/tools/papyrus-headers"
& "$PSScriptRoot/tools/caprica/Caprica.exe" "$PSScriptRoot/Data/Scripts/Source" --game skyrim --ignorecwd --import $imports --flags "$PSScriptRoot/tools/TESV_Papyrus_Flags.flg" --output "$PSScriptRoot/Data/Scripts"
if($LASTEXITCODE -ne 0){throw 'Papyrus build failed'}
python "$PSScriptRoot/tools/make_plugin.py"
if($LASTEXITCODE -ne 0){throw 'ESP generation failed'}
python "$PSScriptRoot/tools/package.py"
if($LASTEXITCODE -ne 0){throw 'Packaging failed'}
