cmake_minimum_required(VERSION 3.14)

# https://stackoverflow.com/a/50360945/8242705
# https://code.visualstudio.com/docs/cpp/faq-cpp#_how-do-i-get-intellisense-to-work-correctly
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND VCPKG_FEATURE_FLAGS "versions")
# Setup vcpkg script with CMake (note: should be placed before project() call)
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
else()
    set(CMAKE_TOOLCHAIN_FILE "./vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

set(PROJECT "marching-cubes")
project(${PROJECT} LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/5.15.2/msvc2019_64")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}QuickCompiler)
message(STATUS "QT_FOUND ${QT_FOUND}")
message(STATUS "QT_CONFIG ${QT_CONFIG}")
message(STATUS "QT_CONSIDERED_CONFIGS ${QT_CONSIDERED_CONFIGS}")
message(STATUS "QT_CONSIDERED_VERSIONS ${QT_CONSIDERED_VERSIONS}")
message(STATUS "Qt5Qml_INCLUDE_DIRS ${Qt5Qml_INCLUDE_DIRS}")
message(STATUS "QT_VERSION_MAJOR ${QT_VERSION_MAJOR}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package(...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

file(GLOB_RECURSE SRC_LIST RELATIVE ${CMAKE_SOURCE_DIR}
    "src/*.cpp"
    "src/*.qrc")
# https://stackoverflow.com/a/57928919/8242705
file(GLOB_RECURSE HEADER_LIST RELATIVE ${CMAKE_SOURCE_DIR}
    "src/*.h")
foreach(header_path ${HEADER_LIST})
    GET_FILENAME_COMPONENT(header_dir ${header_path} PATH)
    SET(header_dir_list ${header_dir_list} ${header_dir})
endforeach()
LIST(REMOVE_DUPLICATES header_dir_list)
message(STATUS "SRC_LIST: ${SRC_LIST}")
message(STATUS "header_dir_list: ${header_dir_list}")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT}
        ${SRC_LIST}
    )
else()
    if(ANDROID)
        add_library(${PROJECT} SHARED
            ${SRC_LIST}
        )
    else()
        add_executable(${PROJECT}
          ${SRC_LIST}
        )
    endif()
endif()

target_compile_definitions(${PROJECT}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT}
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick)


find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE unofficial::sqlite3::sqlite3)

target_include_directories(${PROJECT}
    PRIVATE
    ${header_dir_list})
