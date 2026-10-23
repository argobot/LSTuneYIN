# define installer name
Name "LSTune"
Caption "LS Tune Virtual Strobe Tuner"
outFile "LSTune_Win32_Installer_v0_2.exe"

# Paranoia 
CRCCheck on

# InstallDir "$PROGRAMFILES\LSTune"
InstallDir "$PROGRAMFILES\LSTune"

LicenseText "LS Tune License Information"
LicenseData "..\license.txt"

RequestExecutionLevel admin

ShowInstDetails show

# set desktop as install directory
# InstallDir $DESKTOP
 
# default section start
section
 
# define output path
setOutPath $INSTDIR
 
# specify file to go in output path
#directory LSTune
file "LSTune\*.*"
 
# define uninstaller name
writeUninstaller $INSTDIR\uninstaller.exe
 
# Create start menu shortcut
createDirectory "$SMPROGRAMS\LSTune"
createShortCut "$SMPROGRAMS\LSTune\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
createShortCut "$SMPROGRAMS\LSTune\LSTune.lnk" "$INSTDIR\lstune.exe"
createShortCut "$DESKTOP\LS Tune.lnk" "$INSTDIR\lstune.exe"

# default section end
sectionEnd
 
# create a section to define what the uninstaller does.
# the section will always be named "Uninstall"
section "Uninstall"
 
# Always delete uninstaller first
delete $INSTDIR\uninstaller.exe
 
# now delete installed file
delete "$DESKTOP\LS Tune.lnk"
delete "$INSTDIR\lstune.exe"
delete "$INSTDIR\QtCore4.dll"
delete "$INSTDIR\QtGui4.dll"
delete "$INSTDIR\QtMultimedia4.dll"
delete "$INSTDIR\libgcc_s_dw2-1.dll"
delete "$INSTDIR\mingwm10.dll"
rmDir "$INSTDIR";



delete "$SMPROGRAMS\LSTune\Uninstall.lnk"
delete "$SMPROGRAMS\LSTune\LSTune.lnk"
RMDir "$SMPROGRAMS\LSTune"

sectionEnd