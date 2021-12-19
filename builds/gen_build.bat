@echo off
setlocal enabledelayedexpansion
set devenv="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe"
set lower=0
set vn=1
echo Finding upper bound...

:upper_bound
if exist v!vn!.zip (
	echo v!vn!.zip exists
	set lower=!vn!
	set /a vn*=2
	goto upper_bound
)
echo v!vn!.zip does not exist
echo Finding next build number...

:find_build
set /a targetvn=lower+1
if !vn! equ !targetvn! goto build_installers
set /a midvn=(lower+vn)/2
if exist v!midvn!.zip (
	echo v!midvn!.zip exists
	set lower=!midvn!
) else (
	echo v!midvn!.zip does not exist
	set vn=!midvn!
)
goto find_build

:build_installers
echo(
mkdir v!vn!_temp
pushd ..\installer
for %%s in (..\Release\*.scr) do (
	set screensaver=%%s
	set screensaver=!screensaver:~11,-4!
	for /f "tokens=* delims=" %%l in ('findstr IDS_DESCRIPTION ..\!screensaver!\!screensaver!.rc') do set title=%%l
	set title=!title:*"=!
	set title=!title:~0,-1!
	for /f "tokens=1-3 delims={}" %%l in ('findstr ProjectGuid ..\!screensaver!\!screensaver!.vcxproj') do set id={%%m}

	echo Building !screensaver! installer...
	type NUL > installer.vdproj
	for /f "tokens=* delims=" %%m in (installer-template.vdproj) do (
		set line=%%m
		call set line=%%line:[screensaver]=!screensaver!%%
		call set line=%%line:[title]=!title!%%
		call set line=%%line:[version]=!vn!%%
		call set line=%%line:[id]=!id!%%
		>> installer.vdproj echo !line!
	)
	type NUL > Release\log.txt
	!devenv! installer.vdproj /rebuild Release /out Release\log.txt
	set error=error
	for /f "tokens=* delims=" %%l in ('findstr /C:"1 succeeded" Release\log.txt') do set error=
	if !error!==error (
		echo(
		type Release\log.txt
		echo(
		goto end
	)
	add-install-location.vbs C:\Windows\!screensaver!.scr
	move Release\installer.msi ..\builds\v!vn!_temp\!screensaver!_installer.msi
)
popd

:gen_build
echo(
echo Generating build v!vn!.zip...
powershell Compress-Archive ..\Release\*.scr, v!vn!_temp\* v!vn!.zip
echo Generated build v!vn!.zip
rmdir v!vn!_temp /s /q

:end
pause
