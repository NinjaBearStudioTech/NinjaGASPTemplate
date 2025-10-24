@echo off
setlocal EnableDelayedExpansion

echo === Refreshing Ninja Bear Studio plugins ===

REM Set the Plugins folder path relative to this script
set "PLUGIN_DIR=%~dp0..\Plugins\NinjaBearStudio"

REM List of plugin folder names (must match cloned repo folders)
set PLUGINS=NinjaCombat NinjaCommonUI NinjaGAS NinjaFactions NinjaInput NinjaInteraction NinjaInventory NinjaProfile NinjaPerception NinjaTokens

REM Change to plugin directory
cd /d "%PLUGIN_DIR%"

for %%P in (%PLUGINS%) do (
    if exist "%%P\.git" (
        echo Updating %%P...
        pushd %%P >nul

        git pull
        if !errorlevel! neq 0 (
            echo [ERROR] Failed to update %%P
        )

        popd >nul
    ) else (
        echo Skipping %%P - not a valid git repo
    )
)

echo === Plugin refresh complete ===
endlocal
