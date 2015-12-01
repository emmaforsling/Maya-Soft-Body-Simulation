# This modual is created utilizing the tutorial on https://www.youtube.com/watch?v=2mUOt_F2ywo

# SPECIFY THE DEFAULT MAYA VERSION
if(NOT DEFINED MAYA_VERSION)
	set(MAYA_VERSION 2016 CACHE STRING "Maya version")
endif()

# SPECIFY THE LOCATION OF WHERE MAYA IS INSTALLED, FOR EACH PLATTFORM
set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL")
set(MAYA_INSTALL_BASE_SUFFIX "")
set(MAYA_INCLUDE_SUFFIX "include")
set(MAYA_LIB_SUFFIX "lib")
set(MAYA_BIN_SUFFIX "bin")
set(MAYA_TARGET_TYPE LIBRARY)


if(WIN32)
	# Windows
	set(MAYA_INSTALL_BASE_DEFAULT "C:/Program Files/Autodesk")
	set(MAYA_COMPILE_DEFINITIONS "${MAYA_COMPILE_DEFINITIONS};NT_PLUGIN")
	set(OPENMAYA OpenMaya.lib)
	set(MAYA_PLUGIN_EXTENSION ".mll")
	set(MAYA_TARGET_TYPE RUNTIME)
elseif(APPLE)
	# mac
	set(MAYA_INSTALL_BASE_DEFAULT "/Applications/Autodesk")
	set(MAYA_INCLUDE_SUFFIX "devkit/include")
	set(MAYA_LIB_SUFFIX "Maya.app/Contents/MacOS")
	set(MAYA_BIN_SUFFIX "Maya.app/Contents/bin")
	set(MAYA_COMPILE_DEFINITIONS "${MAYA_COMPILE_DEFINITIONS};OSMac_")
	set(OPENMAYA libOpenMaya.dylib)
	set(MAYA_PLUGIN_EXTENSION ".bundle")
else()
	# linux
	set(MAYA_COMPILE_DEFINITIONS "${MAYA_COMPILE_DEFINITIONS};LINUX")
	set(MAYA_INSTALL_BASE_DEFAULT "/usr/autodesk")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	if(MAYA_VERSION LESS 2016)
		set(MAYA_INSTALL_BASE_SUFFIX -x64)
	endif()
	set(OPENMAYA libOpenMaya.so)
	set(MAYA_PLUGIN_EXTENSION ".so")
endif()

# STORE THE LOCATION (OF WHERE MAYA IS INSTALLED) AS THE VARIABLE MAYA_LOCATION
# set(variable toThisVariable)
set(MAYA_INSTALL_BASE_PATH ${MAYA_INSTALL_BASE_DEFAULT} CACHE STRING "Root Maya Installation Path")
set(MAYA_LOCATION ${MAYA_INSTALL_BASE_PATH}/maya${MAYA_VERSION}${MAYA_INSTALL_BASE_SUFFIX})

# Access the library directory (path)
find_path(MAYA_LIBRARY_DIR ${OPENMAYA}
	PATHS
		${MAYA_LOCATION}
		$ENV{MAYA_LOCATION}
	PATH_SUFFIXES
		"${MAYA_LIB_SUFFIX}/"
	DOC
		"MAYA library path"

)

# SEARCH FOR EACH INDIVIDUELL LIBRARY
# Access the header directory (include map)
find_path(MAYA_INCLUDE_DIR maya/mFn.h
	PATHS
		${MAYA_LOCATION}
		$ENV{MAYA_LOCATION}
	PATH_SUFFIXES
		"${MAYA_INCLUDE_SUFFIX}/"
	DOC
		"MAYA library path"
)

#Access individuell libraries
set(_MAYA_LIBRARIES OpenMaya OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI Foundation)
foreach(MAYA_LIB ${_MAYA_LIBRARIES} )
	find_library(MAYA_${MAYA_LIB}_LIBRARY NAMES ${MAYA_LIB} PATHS ${MAYA_LIBRARY_DIR} NO_DEFAULT_PATH)
	set(MAYA_LIBRARIES ${MAYA_LIBRARIES} ${MAYA_${MAYA_LIB}_LIBRARY})
endforeach()

# WRAP UP THE MODULE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Maya DEFAULT_MSG MAYA_INCLUDE_DIR MAYA_LIBRARIES)

function(MAYA_PLUGIN _target)
	if(WIN32)
		set_target_properties(${_target} PROPERTIES
			LINK_FLAGS "export:initializePlugin /export:uninitializePlugin"
		)
	endif()
	set_target_properties(${_target} PROPERTIES
		COMPILE_DEFINITIONS "${MAYA_COMPILE_DEFINITIONS}"
		PREFIX ""
		SUFFIX ${MAYA_PLUGIN_EXTENSION}
	)
endfunction()

