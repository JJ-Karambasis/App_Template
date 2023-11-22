# Project setup
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(Project_One)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)


# Root path
set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../code)

# Build shaders
set(DATA_PATH "${PROJECT_ROOT_PATH}/../data")
set(SCRIPT_PATH "${PROJECT_ROOT_PATH}/shaders")
include(${SCRIPT_PATH}/build_shaders.cmake)

# Android libraries
find_library(log-lib log)
find_library(android-lib android)
link_libraries(${log-lib} ${android-lib})

# Default compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-unused-variable -Wno-unused-result -fdiagnostics-absolute-paths")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_BUILD")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Vulkan Loader Library (required for vulkan renderers)
add_library(vk-loader SHARED ${PROJECT_ROOT_PATH}/vk_loader/vk_android_loader.cpp)
target_include_directories(vk-loader PRIVATE ${PROJECT_ROOT_PATH}/dependencies/Vulkan-Headers/include)

# Common include directory
include_directories(${PROJECT_ROOT_PATH})
include_directories(${PROJECT_ROOT_PATH}/core)
include_directories(${PROJECT_ROOT_PATH}/platform)
include_directories(${PROJECT_ROOT_PATH}/app)
include_directories(${PROJECT_ROOT_PATH}/renderer)

# Renderer Library
add_library(app-renderer SHARED ${PROJECT_ROOT_PATH}/renderer/vk/vk_renderer.cpp)
target_include_directories(app-renderer PRIVATE ${PROJECT_ROOT_PATH}/vk_loader ${PROJECT_ROOT_PATH}/dependencies/Vulkan-Headers/include ${PROJECT_ROOT_PATH}/shaders)
target_link_libraries(app-renderer vk-loader)

# App Library
add_library(app SHARED ${PROJECT_ROOT_PATH}/app/app.cpp)
target_link_libraries(app)

# Native Activity Glue Library
add_library(android-native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

if(TEST_BUILD)
    # Native Activity Library for test build
    add_library(native-activity SHARED ${PROJECT_ROOT_PATH}/platform/android/android_test.cpp)
    target_include_directories(native-activity PRIVATE ${PROJECT_ROOT_PATH}/tests
                               ${PROJECT_ROOT_PATH}/dependencies/googletest/googletest/include
                               ${PROJECT_ROOT_PATH}/dependencies/googletest/googletest/)

    # Copy libraries to android app_test libraries
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/app_test/src/main)

    # Copy shaders to android app_test assets directory
    file(COPY ${DATA_PATH}/shaders DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/app_test/src/main/assets)

else()
    # Native Activity Library for normal app build
    add_library(native-activity SHARED ${PROJECT_ROOT_PATH}/platform/android/android_app.cpp)

    # Copy libraries to android app libraries
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main)

    # Copy shaders to android app assets directory
    file(COPY ${DATA_PATH}/shaders DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets)

endif()

target_include_directories(native-activity PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue)
target_link_libraries(native-activity android-native-app-glue)