# PowerShell setup script for lixxuid dependencies

$depsDir = Join-Path $PSScriptRoot "deps"
if (!(Test-Path $depsDir)) {
    New-Item -ItemType Directory -Path $depsDir | Out-Null
}

# 1. MinHook
$minhookDir = Join-Path $depsDir "minhook"
if (!(Test-Path $minhookDir)) {
    Write-Host "[*] Fetching MinHook..." -ForegroundColor Cyan
    git clone https://github.com/TsudaKageyu/minhook.git $minhookDir
} else {
    Write-Host "[+] MinHook already present." -ForegroundColor Green
}

# 2. ImGui (docking branch or master)
$imguiDir = Join-Path $depsDir "imgui"
if (!(Test-Path $imguiDir)) {
    Write-Host "[*] Fetching Dear ImGui..." -ForegroundColor Cyan
    git clone -b docking https://github.com/ocornut/imgui.git $imguiDir
} else {
    Write-Host "[+] ImGui already present." -ForegroundColor Green
}

Write-Host "`n[+] Dependencies are set up in $depsDir." -ForegroundColor Green
