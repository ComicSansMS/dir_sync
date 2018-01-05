if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(QT_DIR_SUFFIX "clang_64")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(MSVC_VERSION EQUAL 1900)
        set(QT_DIR_SUFFIX "msvc2015_64")
    elseif((MSVC_VERSION EQUAL 1910) OR (MSVC_VERSION EQUAL 1911) OR (MSVC_VERSION EQUAL 1912))
        set(QT_DIR_SUFFIX "msvc2017_64")
    endif()
endif()

find_path(QT5_BASE_DIR
    NAMES "lib/cmake/Qt5"
    HINTS ENV QT5_ROOT
    PATH_SUFFIXES "5.8/${QT_DIR_SUFFIX}" "5.9/${QT_DIR_SUFFIX}" "5.10.0/${QT_DIR_SUFFIX}"
    DOC "Qt5 Root Directory"
)

if(WIN32)
    function(getPDBForDLL DLL_PATH OUT_VAR)
        get_filename_component(dll_dir ${DLL_PATH} DIRECTORY)
        get_filename_component(dll_we ${DLL_PATH} NAME_WE)
        set(${OUT_VAR} "${dll_dir}/${dll_we}.pdb" PARENT_SCOPE)
    endfunction()


    function(getQt5Dlls QT_TARGET OUT_VAR)
        unset(DLLS)
        get_property(tmp TARGET ${QT_TARGET} PROPERTY IMPORTED_LOCATION_DEBUG)
        list(APPEND DLLS ${tmp})
        getPDBForDLL(${tmp} tmp_pdb)
        if (EXISTS ${tmp_pdb})
            list(APPEND DLLS ${tmp_pdb})
        endif()
        get_property(tmp TARGET ${QT_TARGET} PROPERTY IMPORTED_LOCATION_RELEASE)
        list(APPEND DLLS ${tmp})
        get_property(deps TARGET ${QT_TARGET} PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG)
        foreach(dep ${deps})
            if(TARGET ${dep})
                getQt5Dlls(${dep} tmp)
                list(APPEND DLLS ${tmp})
            endif()
        endforeach()
        set(result ${${OUT_VAR}})
        list(APPEND result ${DLLS})
        list(REMOVE_DUPLICATES result)
        set(${OUT_VAR} ${result} PARENT_SCOPE)
    endfunction()
endif()
