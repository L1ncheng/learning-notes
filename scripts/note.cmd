@echo off
rem Daily study-log helper (Windows / cmd / double-click friendly).
rem
rem   scripts\note.cmd                 create today's note
rem   scripts\note.cmd 2026-09-15      backfill a specific date
rem   scripts\note.cmd yesterday       backfill yesterday
rem   scripts\note.cmd --index         rebuild the index only
rem
rem Requires Node.js 18+  ->  https://nodejs.org

setlocal
set "SCRIPT_DIR=%~dp0"

where node >nul 2>nul
if errorlevel 1 (
    echo [X] Node.js not found. Install Node.js 18+ from https://nodejs.org
    exit /b 127
)

if "%~1"=="--index" (
    node "%SCRIPT_DIR%build-index.mjs" %2 %3 %4 %5
    exit /b %errorlevel%
)

node "%SCRIPT_DIR%new-note.mjs" %*
exit /b %errorlevel%
