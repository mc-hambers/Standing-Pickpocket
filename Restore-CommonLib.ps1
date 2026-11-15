param([Parameter(Mandatory=$true)][string]$CommonLibArchive)
$ErrorActionPreference = 'Stop'
# Use the official v12.1.0 all-msvc-cmake release ZIP. Its required original
# library hash is checked before restoring it. No executable is launched.
$unpack = Join-Path $PSScriptRoot 'build/commonlib-restore'
New-Item -ItemType Directory -Force $unpack | Out-Null
Expand-Archive -LiteralPath $CommonLibArchive -DestinationPath $unpack -Force
$candidate = Get-ChildItem -LiteralPath $unpack -Recurse -Filter CommonLibSSE.lib | Where-Object { (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash -eq '666eb00b048390e3bbb0f39865e9774c0eb8bdad21043997d9e581933b3fbba1' } | Select-Object -First 1
if (!$candidate) { throw 'The archive does not contain the matching CommonLibSSE library.' }
$destination = Join-Path $PSScriptRoot 'vendor/prebuilt/commonlibsse-ng-prebuilt-v12.1.0-all-msvc-cmake/lib/CommonLibSSE.lib'
New-Item -ItemType Directory -Force (Split-Path $destination) | Out-Null
Copy-Item -LiteralPath $candidate.FullName -Destination $destination -Force
Write-Output 'Matching CommonLib static library restored. Run Build-All.ps1 with your MSVC toolchain.'
