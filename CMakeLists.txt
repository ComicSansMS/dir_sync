cmake_minimum_required(VERSION 3.9)

project(dir_sync LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    option(MULTIPROCESS_BUILD "Enables the /MP compile flag for Visual C++." ON)
    if(MULTIPROCESS_BUILD)
        set(MULTIPROCESS_LIMIT_BUILD_PROCESSES 0 CACHE STRING
            "If set to a positive number, this will be used as an argument to /MP.")
        if(NOT MULTIPROCESS_LIMIT_BUILD_PROCESSES)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${MULTIPROCESS_LIMIT_BUILD_PROCESSES}")
        endif()
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Qt5BaseDir)
if(QT5_BASE_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${QT5_BASE_DIR})
endif()
find_package(Qt5Widgets REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED filesystem)
find_package(gbBase REQUIRED)

set(DIR_SYNC_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(DIR_SYNC_SOURCE_FILES
    ${DIR_SYNC_SOURCE_DIR}/main.cpp
    ${DIR_SYNC_SOURCE_DIR}/config.cpp
)

set(DIR_SYNC_HEADER_FILES
    ${DIR_SYNC_SOURCE_DIR}/config.hpp
    ${DIR_SYNC_SOURCE_DIR}/exceptions.hpp
)

set(DIR_SYNC_UI_DIR ${PROJECT_SOURCE_DIR}/src/ui)
set(DIR_SYNC_QT_SOURCE_FILES
    ${DIR_SYNC_UI_DIR}/main_window.cpp
)
set(DIR_SYNC_QT_MOC_HEADER_FILES
    ${DIR_SYNC_UI_DIR}/main_window.hpp
)
qt5_wrap_cpp(DIR_SYNC_QT_MOC_SOURCE_FILES ${DIR_SYNC_QT_MOC_HEADER_FILES})
source_group(moc FILES
    ${DIR_SYNC_QT_MOC_SOURCE_FILES}
)

add_executable(dir_sync WIN32
    ${DIR_SYNC_SOURCE_FILES}
    ${DIR_SYNC_HEADER_FILES}
    ${DIR_SYNC_QT_SOURCE_FILES}
    ${DIR_SYNC_QT_MOC_HEADER_FILES}
    ${DIR_SYNC_QT_MOC_SOURCE_FILES}
)
target_include_directories(dir_sync PUBLIC ${DIR_SYNC_SOURCE_DIR})
target_link_libraries(dir_sync PUBLIC Qt5::Widgets Ghulbus::gbBase Boost::disable_autolinking Boost::filesystem)
if(WIN32)
    target_link_libraries(dir_sync PUBLIC Qt5::WinMain)
endif()
target_compile_options(dir_sync PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4>)
target_compile_options(dir_sync PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/permissive->)
#target_compile_options(dir_sync PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/std:c++latest>)
target_compile_options(dir_sync PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)
target_compile_options(dir_sync PRIVATE $<$<CXX_COMPILER_ID:GNU>:-pedantic>)
target_compile_options(dir_sync PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wextra>)
target_compile_definitions(dir_sync PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT dir_sync)

if(WIN32)
    getQt5Dlls(Qt5::Widgets qt_DLLS)
    file(COPY ${qt_DLLS} ${QT5_ADDITIONAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
endif()
