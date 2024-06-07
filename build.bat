@echo off

set workbench="F:\SteamLibrary\steamapps\common\Arma Reforger Experimental Tools\Workbench\ArmaReforgerWorkbenchSteamDiag.exe"

set reforgerPath=F:\SteamLibrary\steamapps\common\Arma Reforger Experimental\addons\
set addonToPack=ArmaForces_Mods_Core

%workbench% -enableWARP -exitAfterInit -noThrow -noSound -wbModule=ResourceManager ^
    -profile "%cd%/_temp/profile" ^
    -packAddon -packAddonDir "%cd%/_temp/pack" ^
    -addonsDir "%reforgerPath%,%cd%/addons/" ^
    -addons %addonToPack%

if %ERRORLEVEL% NEQ 0 echo Error
