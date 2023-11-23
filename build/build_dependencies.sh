SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

git submodule update --init --recursive

DXC_Path="$SCRIPTPATH/../code/dependencies/DirectXShaderCompiler"

if [ ! -f "$DXC_Path/bin/bin/dxc" ]; then

    cmake -C $DXC_Path/cmake/caches/PredefinedParams.cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SPIRV_CODEGEN=1 -S $DXC_Path -B $DXC_Path/bin
    cmake --build $DXC_Path/bin

fi

if [ ! -d "$SCRIPTPATH/xcode/vulkan.framework" ]; then
    cp -r "$VULKAN_SDK/Frameworks/vulkan.framework" "$SCRIPTPATH/xcode/vulkan.framework"
fi

if [ ! -f "$SCRIPTPATH/xcode/libraries/macOS/libMoltenVK.dylib" ]; then
    mkdir -p "$SCRIPTPATH/xcode/libraries/macOS/"
    cp -r "$VULKAN_SDK/../MoltenVK/dylib/macOS/libMoltenVK.dylib" "$SCRIPTPATH/xcode/libraries/macOS/libMoltenVK.dylib"
fi