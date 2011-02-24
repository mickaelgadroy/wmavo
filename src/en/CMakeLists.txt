
project(WmEngine)
cmake_minimum_required(VERSION 2.6)

set( plugin_type Engine )
set( plugin_name wmEngine )

set( in_MOC_H src/wmengine.h )
set( in_UIS_H "" )
set( in_RC_SCRS "" )

set( OpenBabel2_include_DIR "/usr/include/openbabel-2.0/" )
set( wmavoconst_include_dir ../ )


# Add a flag in CMake attribut.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -lglut")

# Search files and library.
find_package(Avogadro 1.0.0 REQUIRED)
file(	GLOB_RECURSE source_files	src/* )

## Set up the build environment
include(${Avogadro_USE_FILE})
include_directories(${OpenBabel2_include_DIR})
include_directories( ${wmavoconst_include_dir} )

## Génération des fichiers nécessaire pour la compilation 
## et éditions de liens avec Qt, Avogadro & Co.
QT4_WRAP_CPP(out_MOC_H ${in_MOC_H})
qt4_wrap_ui(out_UIS_H ${in_UIS_H})
qt4_add_resources(out_RC_SRCS ${in_RC_SCRS})

add_library( ${plugin_name} MODULE ${source_files} ${out_UIS_H} ${out_MOC_H} ${out_RC_SRCS} )
target_link_libraries( ${plugin_name} avogadro )
#target_link_libraries( ${plugin_name} GL GLU glut )

# ?
#install(TARGETS ${plugin_name} DESTINATION "${Avogadro_PLUGIN_DIR}/${plugin_type}")
#set_target_properties(${plugin_name} PROPERTIES OUTPUT_NAME ${plugin_name} PREFIX "")

