SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

cmake -DSCRIPT_PATH=$SCRIPTPATH/../../code/shaders -DDATA_PATH=$SCRIPTPATH/../../data -P $SCRIPTPATH/../../code/shaders/build_shaders.cmake 