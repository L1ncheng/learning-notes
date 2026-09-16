# 学习日志快捷入口（Windows / PowerShell 7+）
#
#   .\scripts\note.ps1                  创建今天的笔记
#   .\scripts\note.ps1 2026-09-15       补写指定日期
#   .\scripts\note.ps1 yesterday        补写昨天
#   .\scripts\note.ps1 -Force           覆盖已存在的笔记
#   .\scripts\note.ps1 -IndexOnly       只重建索引

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$Date = '',

    [switch]$Force,
    [switch]$IndexOnly,
    [switch]$Quiet,
    [switch]$Help
)

$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$repoRoot = Split-Path -Parent $PSScriptRoot
$nodeExe = (Get-Command node -ErrorAction SilentlyContinue).Source

if ($Help) {
    Write-Host @"
用法: .\scripts\note.ps1 [日期] [-Force] [-IndexOnly] [-Quiet]

  日期        YYYY-MM-DD、today（默认）或 yesterday
  -Force      已存在时覆盖
  -IndexOnly  只重建 INDEX.md 与 README 统计
  -Quiet      精简输出

需要 Node.js 18+（https://nodejs.org）。
"@
    exit 0
}

if (-not $nodeExe) {
    Write-Host '✗ 找不到 node。请先安装 Node.js 18+：https://nodejs.org' -ForegroundColor Red
    exit 127
}

$nodeArgs = @()

if ($IndexOnly) {
    $nodeArgs += (Join-Path $repoRoot 'scripts\build-index.mjs')
} else {
    $nodeArgs += (Join-Path $repoRoot 'scripts\new-note.mjs')
    if ($Date) { $nodeArgs += $Date }
}

if ($Force) { $nodeArgs += '--force' }
if ($Quiet) { $nodeArgs += '--quiet' }

& $nodeExe @nodeArgs
exit $LASTEXITCODE
