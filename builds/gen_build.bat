@echo off
set lower=0
set vn=1
echo Finding upper bound...

:upper_bound
if exist v%vn%.zip (
	echo v%vn%.zip exists
	set lower=%vn%
	set /a vn*=2
	goto upper_bound
)
echo v%vn%.zip does not exist
echo Finding next build number...

:find_build
set /a targetvn=lower+1
if %vn% equ %targetvn% goto gen_build
set /a midvn=(lower+vn)/2
if exist v%midvn%.zip (
	echo v%midvn%.zip exists
	set lower=%midvn%
) else (
	echo v%midvn%.zip does not exist
	set vn=%midvn%
)
goto find_build

:gen_build
echo Generating build v%vn%.zip...
powershell Compress-Archive ../Release/*.scr v%vn%.zip
echo Generated build v%vn%.zip
pause
