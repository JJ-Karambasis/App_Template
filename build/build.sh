Script=$(realpath "$0")
Script_Path=$(dirname "$Script")

if [ "$1" = "osx" ]; then
    pushd "$Script_Path/xcode"
        xcodebuild -quiet -scheme osx-app build
    popd
fi

if [ "$1" = "ios" ]; then
    pushd "$Script_Path/xcode"
        xcodebuild -quiet -scheme ios-app build
    popd
fi