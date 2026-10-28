$ErrorActionPreference = "Stop"
$src = Join-Path $PSScriptRoot "src"
$out = Join-Path $PSScriptRoot "out"
New-Item -ItemType Directory -Force -Path $out | Out-Null

$gpp = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gpp) {
    throw "Не знайдено g++. Встановіть MinGW з POSIX-потоками і додайте його bin до PATH."
}

Get-ChildItem $src -Filter "*.cpp" | Sort-Object Name | ForEach-Object {
    $exe = Join-Path $out ($_.BaseName + ".exe")
    & g++ -std=c++17 -O2 -pthread $_.FullName -o $exe
    if ($LASTEXITCODE -ne 0) {
        throw "Помилка збірки $($_.Name)"
    }
    Write-Host "OK $($_.Name)"
}
