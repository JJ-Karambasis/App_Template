set(DXC_PATH ${SCRIPT_PATH}/../dependencies/DirectXShaderCompiler)
string(FIND "${DXC_PATH}" "C:/" out)
if("${out}" STREQUAL "0")
    set(IS_WINDOWS_PATH TRUE)
else()
    set(IS_WINDOWS_PATH FALSE)
endif()

if(NOT IS_WINDOWS_PATH)
    string(FIND "${DXC_PATH}" "C:\\" out)
    if("${out}" STREQUAL "0")
        set(IS_WINDOWS_PATH TRUE)
    else()
        set(IS_WINDOWS_PATH FALSE)
    endif()
endif()


if(IS_WINDOWS_PATH)
    find_program(DXC dxc.exe FILES PATHS ${DXC_PATH}/bin/Release/bin NO_DEFAULT_PATH NO_CACHE)
    if(NOT DXC)
        message(FATAL_ERROR "Cannot find 'dxc.exe'. Please run build_dependencies.bat in {PROJECT_DIR}/build")
    endif()
else()
    find_program(DXC dxc FILES PATHS ${DXC_PATH}/bin/bin NO_DEFAULT_PATH NO_CACHE)
    if(NOT DXC)
        message(FATAL_ERROR "Cannot find 'dxc'. Please run build_dependencies.sh in {PROJECT_DIR}/build")
    endif()
endif()

set(shared_shader_options -nologo -Qembed_debug -Zi -spirv)
set(vtx_shader_options ${shared_shader_options} -T vs_6_0 -E VS_Main)
set(pxl_shader_options ${shared_shader_options} -T ps_6_0 -E PS_Main)

file(MAKE_DIRECTORY ${DATA_PATH}/shaders/)

execute_process(
    COMMAND ${DXC} ${vtx_shader_options} /Fo ${DATA_PATH}/shaders/app_vs.shader ${SCRIPT_PATH}/app.hlsl
    COMMAND ${DXC} ${pxl_shader_options} /Fo ${DATA_PATH}/shaders/app_ps.shader ${SCRIPT_PATH}/app.hlsl
)