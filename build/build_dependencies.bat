@echo off

git submodule update --init --recursive

set DXC_Path=%~dp0..\code\dependencies\DirectXShaderCompiler
IF NOT EXIST %DXC_Path%\bin\Release\bin\dxc.exe (
    call %DXC_Path%\utils\hct\hctstart.cmd %DXC_Path%\ %DXC_Path%\bin
    python %DXC_Path%\utils\hct\hctgettaef.py
    call %DXC_Path%\utils\hct\hctbuild.cmd -spirv -rel
)