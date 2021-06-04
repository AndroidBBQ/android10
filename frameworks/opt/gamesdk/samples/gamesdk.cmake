set( _MY_DIR ${CMAKE_CURRENT_LIST_DIR})

# This function will create a static library target called 'gamesdk'.
# The location of the library is set according to your ANDROID_NDK_REVISION
# and ANDROID_PLATFORM, unless you explicitly set GAMESDK_NDK_VERSION
#   GAMESDK_ANDROID_SDK_VERSION or pass these as 4th and 5th arguments.
# Optional arguments, in order:
#  PACKAGE_DIR: where the packaged version of the library is, relative to the gamesdk root dir
#    default value: package/localtf
#  GAMESDK_GEN_TASK: the gradle task to build the package
#    default value: localTfBuild
#  GAMESDK_DO_BUILD: whether to add a custom build command to build the gamesdk (ON/OFF)
#    default value: ON
#  GAMESDK_NDK_VERSION: version number for the NDK (major.minor)
#    default value: derived from ANDROID_NDK_REVISION
#  GAMESDK_ANDROID_SDK_VERSION: android min SDK level
#    default value: derived from ANDROID_PLATFORM
# Each argument can be specified using a variable instead of being passed in.
function(add_gamesdk_target)
	if(ARGC GREATER 0)
		if(NOT ${ARGV0} STREQUAL "")
			set(GAMESDK_PACKAGE_DIR ${ARGV0})
		endif()
	endif()
	if(ARGC GREATER 1)
		if(NOT ${ARGV1} STREQUAL "")
			set(GAMESDK_GEN_TASK ${ARGV1})
		endif()
	endif()
	if(ARGC GREATER 2)
		if(NOT ${ARGV2} STREQUAL "")
			set(GAMESDK_DO_BUILD ${ARGV2})
		endif()
	endif()
	if(ARGC GREATER 3)
		if(NOT ${ARGV3} STREQUAL "")
			set(GAMESDK_NDK_VERSION "${ARGV3}")
		endif()
	endif()
	if(ARGC GREATER 4)
		set(GAMESDK_ANDROID_SDK_VERSION "${ARGV4}")
    endif()
    if(NOT DEFINED GAMESDK_DO_BUILD)
		set(GAMESDK_DO_BUILD ON)
    endif()
    if(NOT DEFINED GAMESDK_PACKAGE_DIR)
		set(GAMESDK_PACKAGE_DIR "package/localtf")
    endif()
    if(NOT DEFINED GAMESDK_GEN_TASK)
		set(GAMESDK_GEN_TASK "localTfBuild")
    endif()
    if (NOT DEFINED GAMESDK_NDK_VERSION)
		string(REGEX REPLACE "^([^.]+)[.]([^.]+).*" "\\1.\\2" GAMESDK_NDK_VERSION ${ANDROID_NDK_REVISION} )
    endif()
    if (NOT DEFINED GAMESDK_ANDROID_SDK_VERSION)
		string(REGEX REPLACE "^android-([^.]+)" "\\1" GAMESDK_ANDROID_SDK_VERSION ${ANDROID_PLATFORM} )
    endif()
    set(GAMESDK_PACKAGE_DIR "${_MY_DIR}/../../${GAMESDK_PACKAGE_DIR}")
    set(BUILD_NAME ${ANDROID_ABI}_SDK${GAMESDK_ANDROID_SDK_VERSION}_NDK${GAMESDK_NDK_VERSION}_${ANDROID_STL})
    set(GAMESDK_LIB_DIR "${GAMESDK_PACKAGE_DIR}/libs/${BUILD_NAME}")

    include_directories( "${GAMESDK_PACKAGE_DIR}/include" ) # Games SDK Public Includes
    get_filename_component(DEP_LIB "${GAMESDK_LIB_DIR}/libgamesdk.a" REALPATH)
    get_directory_property(hasParent PARENT_DIRECTORY)
    if(hasParent)
        set(GAMESDK_LIB ${DEP_LIB} PARENT_SCOPE)
    endif()

    add_library( gamesdk STATIC IMPORTED GLOBAL)
    if(GAMESDK_DO_BUILD)
        add_custom_command(OUTPUT ${DEP_LIB}
            COMMAND ./gradlew ${GAMESDK_GEN_TASK} -PGAMESDK_ANDROID_SDK_VERSION=${GAMESDK_ANDROID_SDK_VERSION} VERBATIM
            WORKING_DIRECTORY "${_MY_DIR}/.." )
        add_custom_target(gamesdk_lib DEPENDS ${DEP_LIB})
        add_dependencies(gamesdk gamesdk_lib)
    endif()
    set_target_properties(gamesdk PROPERTIES IMPORTED_LOCATION ${DEP_LIB})

endfunction()
