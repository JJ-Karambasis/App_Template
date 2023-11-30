@echo off

set BasePath=%~dp0..

FOR /F "tokens=* USEBACKQ" %%F IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) DO (
    SET MSBuildPath=%%F
)

IF %1 == win32 (
    "%MSBuildPath%" %BasePath%\build\vs\2022\app.sln /p:Configuration=Debug /p:Platform=x64
)

IF %1 == android (

    pushd %BasePath%\build\android\ 
        gradlew.bat assembleDebug
    popd

)
