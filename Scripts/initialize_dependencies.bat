@echo off
setlocal EnableDelayedExpansion

echo === Cloning Ninja Bear Studio plugins ===

:: Set the Plugins folder path relative to this script
set "PLUGIN_DIR=%~dp0..\Plugins\NinjaBearStudio"

:: Create the folder if it doesn't exist
if not exist "%PLUGIN_DIR%" (
    mkdir "%PLUGIN_DIR%"
)

cd /d "%PLUGIN_DIR%"

:: List of primary repositories
set REPO1=https://github.com/NinjaBearStudioTech/NinjaCombat.git
set REPO2=https://github.com/NinjaBearStudioTech/NinjaGAS.git
set REPO3=https://github.com/NinjaBearStudioTech/NinjaFactions.git
set REPO4=https://github.com/NinjaBearStudioTech/NinjaInput.git
set REPO5=https://github.com/NinjaBearStudioTech/NinjaInteraction.git
set REPO6=https://github.com/NinjaBearStudioTech/NinjaInventory.git

for %%R in (1 2 3 4 5 6 7 8 9 10) do (
    call set "REPO=!REPO%%R!"
    for %%A in (!REPO!) do (
        set "REPO_URL=%%A"
        for %%F in (!REPO_URL!) do set "FOLDER=%%~nF"

        if not exist "!FOLDER!" (
            echo Cloning !REPO_URL! into !FOLDER!...
            git clone !REPO_URL!
        ) else (
            echo Folder !FOLDER! already exists. Skipping.
        )
    )
)

echo === Plugin initialization complete ===
endlocal
