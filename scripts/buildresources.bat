call :copyAssets bus
rem call :copyAssets dims
rem call :copyAssets models
rem call :copyAssets osmtiles
rem call :copyAssets raw_osm
rem call :copyAssets tfl_meta
call :copyAssets train_routes
rem call :copyAssets network_rail

goto :eof

:copyAssets
mkdir __obbs
c:\Qt\6.5.3\msvc2019_64\bin\rcc -binary ../%1.qrc -o __obbs/%1.obb

IF ERRORLEVEL 1 (
    cscript //nologo errorpopup.vbs
    EXIT /B 1
)

mkdir ..\android\%1
copy %1\build.gradle ..\android\%1

IF ERRORLEVEL 1 (
    cscript //nologo errorpopup.vbs
    EXIT /B 1
)

mkdir ..\android\%1\src\main\assets
del ..\android\%1\src\main\assets\*.* /q
copy __obbs\%1.obb ..\android\%1\src\main\assets

IF ERRORLEVEL 1 (
    cscript //nologo errorpopup.vbs
    EXIT /B 1
)

