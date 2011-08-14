
set( PLUGIN_NAME wmtool )

# wmextension plugin source files.
set( wmavoconst_include_dir ../ ) # .h (constants & revision)
set( tool_include_dir src/ ) # .h (Tool extension)
set( avo_toolbox_include_dir src/avo_toolbox ) # .h (Avogadro toolbox : Molecule Manipulation, Settigs Widget)
set( qt_extended_include_dir src/qt_extended ) # .h (Extended Qt class)
set( rendering_include_dir src/rendering ) # .h (drawObject & Co)
set( inputdevice_include_dir src/inputdevice src/inputdevice/wiimote ) # .h (Input device : wmDevice ...)
set( wrapper_include_dir src/wrapper src/wrapper/chemical ) # .h (Input device to domain)
set( toolbox_include_dir src/toolbox ) # .h (my toolbox)


set( wiiuse_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../extra/wiiuse_wiiusecpp" ) # root directory of wiiuse/wiiusecpp files.
set( wiiuse_include_dir "${wiiuse_dir}/wiiuse/src" ) # .h
set( wiiusecpp_include_dir "${wiiuse_dir}/wiiusecpp/src" ) # .h
set( wiiuse_lib_name wiiuse )
set( wiiusecpp_lib_name wiiusecpp )
  

if( WIN32 )
# Windows.

  # wiiuse/wiiusecpp libraries.
  set( wiiuse_lib_dir "${wiiuse_dir}/lib_win32" ) # ".lib" for wiise/wiiusecpp (release mode)
  set( wiiuse_libd_dir "${wiiuse_dir}/lib_win32/debug" ) # ".lib" for wiise/wiiusecpp (debug mode)

else() 
# Unix ...
  #set( wiiusecpp_include_dir /home/myck/install/linux/wiiusecpp/src ) # .h
  #set( wiiusecpp_lib_dir /usr/lib ) # ".lib"
  #set( wiiusecpp_lib_name wiiusecpp )
  
  # wiiuse/wiiusecpp libraries.
  set( wiiuse_lib_dir "${wiiuse_dir}/lib_nix64" ) # ".lib" for wiise/wiiusecpp (release mode)
  
  set( OpenBabel2_include_DIR "/usr/include/openbabel-2.0/" )

endif()


if( "${LINK_LIBS}" STREQUAL "avogadro" )
# In the Avogadro compilation process.
# A lot of attribut are initiated for openGL, openBabel ... 
# It is necessary that you (if it is does't already) :
# - add the project in : [avogadro_path]/libavogadro/src/extensions/[here_wmavo_directory]
# - add "add_subdirectory( wmavo/src/tool )" in : [avogadro_path]/libavogadro/src/extensions/CMakeLists.txt

  #set( PLUGIN_LABEL tools )
  #set( PLUGIN_TARGET tools )
  #set( PLUGIN_TYPE tool )
  
  # The Meta-Object Compiler, moc, is the program that handles Qt's C++ extensions.
  set( in_MOC_SRC src/wmtool.cpp )
  set( in_MOC_SRC ${in_MOC_SRC} src/rendering/drawsimpleobject.cpp src/rendering/drawobject.cpp src/rendering/rendertext.cpp src/rendering/distanceanglediedre.cpp)    
  set( in_MOC_SRC ${in_MOC_SRC} src/avo_toolbox/moleculemanipulation.cpp src/avo_toolbox/settingswidget.cpp )
  #set( in_MOC_SRC ${in_MOC_SRC} src/avo_toolbox/undocommand.cpp src/avo_toolbox/constraintsmodel.cpp )
  set( in_MOC_SRC ${in_MOC_SRC} src/inputdevice/inputdevice.cpp src/inputdevice/wmdevice.cpp src/inputdevice/wiimote/wmrumble.cpp )
  #set( in_MOC_SRC ${in_MOC_SRC} src/inputdevice/wiimote/wmavo.cpp ) # Add file to include in project, but it does not need moc file ...
  set( in_MOC_SRC ${in_MOC_SRC} src/wrapper/chemicalwrapper.cpp src/wrapper/chemical/wrapper_chemicalcmd_to_avoaction.cpp src/wrapper/chemical/contextmenu_to_avoaction.cpp )
  set( in_MOC_SRC ${in_MOC_SRC} src/qt_extended/qmenu_ex.cpp src/qt_extended/qaction_ex.cpp src/qt_extended/qundo_ex.cpp )
  
  set( in_UIS_H "" )
  set( in_RC_SCRS src/inputdevice/wiimote/wmtool.qrc )
  
  ## Set up the build environment (to know where are .h, ".dll" and some files)

  # Let to use .h directly : 
  # ex. : #include <wmavo_const.h> instead of <../../wmavo_const.h>
  include_directories( ${wmavoconst_include_dir} )
  include_directories( ${tool_include_dir} )
  include_directories( ${avo_toolbox_include_dir} )
  include_directories( ${inputdevice_include_dir} )
  include_directories( ${qt_extended_include_dir} )
  include_directories( ${rendering_include_dir} )
  include_directories( ${toolbox_include_dir} )
  include_directories( ${wrapper_include_dir} )
  include_directories( ${wiiuse_include_dir} )
  include_directories( ${wiiusecpp_include_dir} )
  
  # ".dll"  directories.
  link_directories( ${wiiuse_lib_dir} )
  link_directories( ${wiiusecpp_lib_dir} )
 
  # Generate moc, specify install directories ...
  avogadro_plugin( ${PLUGIN_NAME} "${in_MOC_SRC}" "${in_UIS_H}" "${in_RC_SCRS}" )

  # Link the LIBRARY ! Not *.h !
  target_link_libraries( ${PLUGIN_NAME} ${wiiuse_lib_name} )
  target_link_libraries( ${PLUGIN_NAME} ${wiiusecpp_lib_name} )
  
  # To install wiiuse/wiiusecpp libraries (& other things)in the Avogadro bin.
  file( GLOB wiiuse_files ${wiiuse_libd_dir}/*wiiuse* )
  if( WIN32 ) 
    install( FILES ${wiiuse_files} DESTINATION ${BIN_INSTALL_DIR} )
   
    #file( GLOB_RECURSE frag_files img/fragments/* )
    #install( FILES ${frag_files} DESTINATION ${BIN_INSTALL_DIR}/../share/avogadro/fragments_resume )
  else()
    install( FILES ${wiiuse_files} DESTINATION "/usr/lib" )
  endif()
  
else()
# In a stand alone compilation process.
# Nothing is initiated ...

  project( ${PLUGIN_NAME} )
  cmake_minimum_required(VERSION 2.6)

  # Avogadro needs.
  if( NOT EXISTS "${Avogadro_DIR}" )
    if( WIN32 )
      #set( Avogadro_DIR "C:/src/avogadro-1.1.0/build" ) # .h
      #set( Avogadro_DIR "C:/Program Files (x86)/Avogadro/bin/avogadro" )
      set( Avogadro_DIR "C:/Program Files (x86)/Avogadro-1.0.3_debug/bin/avogadro" )
      #message( STATUS "Avogadro_DIR is by default : " ${Avogadro_DIR} )
    endif()
  else()
    #message( STATUS "Avogadro_DIR is : " ${Avogadro_DIR} )
  endif()
  
  #message( STATUS "Avogadro directories is searching ..." )
  find_package( Avogadro 1.0.0 REQUIRED )
  message( STATUS "Avogadro directory: ${Avogadro_DIR}" )
  
  # OpenGL needs (Useless, "included" in Avogadro)
  #find_package(OpenGL REQUIRED) # find and setup OpenGL.
  #message( STATUS "OpenGL found : " ${OPENGL_LIBRARIES})

  # Eigen needs.
  if( NOT EXISTS "${EIGEN2_INCLUDE_DIR}" )
    if( WIN32 )
      set( EIGEN2_INCLUDE_DIR "C:/src/eigen-2.0.15" ) # .h
      message( STATUS "EIGEN2_INCLUDE_DIR is by default : " ${EIGEN2_INCLUDE_DIR} )
    endif()
  else()
    message( STATUS "EIGEN2_INCLUDE_DIR is : " ${EIGEN2_INCLUDE_DIR} )
  endif()
  
  # OpenBabel needs.
  #set( OpenBabel2_include_DIR "C:/src/openbabel-2.3.0/include" ) # .h
  #set( OpenBabel22_include_DIR "C:/src/openbabel-2.3.0/windows-vc2008/build/include" ) # .h
  if( NOT EXISTS "${OPENBABEL2_INCLUDE_DIR}" )
    if( WIN32 )
      set( OPENBABEL2_INCLUDE_DIR "C:/src/openbabeld-2.3.0/windows-vc2008/install/include/openbabel-2.0" ) # .h
    else()
      set( OPENBABEL2_INCLUDE_DIR "/usr/include/openbabel-2.0/" )
      
      if( NOT EXISTS "${OPENBABEL2_INCLUDE_DIR}" )
        set( OPENBABEL2_INCLUDE_DIR "/usr/local/include/openbabel-2.0/" )
      endif()
    endif()
  else()
    #message( STATUS "OPENBABEL2_INCLUDE_DIR is : " ${OPENBABEL2_INCLUDE_DIR} )
  endif()
  
  if( EXISTS "${OPENBABEL2_INCLUDE_DIR}" )
    message( STATUS "OPENBABEL2_INCLUDE_DIR is : " ${OPENBABEL2_INCLUDE_DIR} )
  else()
    message( STATUS "OPENBABEL2_INCLUDE_DIR NOT found : " ${OPENBABEL2_INCLUDE_DIR} )
  endif()
  
  # 
  set( LINK_LIBS avogadro )
  set( PLUGIN_LABEL tools )
  set( PLUGIN_TARGET tools )
  set( PLUGIN_TYPE tool )
  
  # glut libraries and source files (Replaced by OpenGL needs)
  #set( glut_INCLUDE_DIR "C:/src/freeglut-2.6.0/include" ) # .h
  #set( glut_LIB_DIR "C:/src/freeglut-2.6.0/Release" ) # .lib
  #set( glut_lib_name freeglut ) # Name of the library to link.
    
  # The Meta-Object Compiler, moc, is the program that handles Qt's C++ extensions.
  set( in_MOC_H src/wmtool.h )
  set( in_MOC_H ${in_MOC_H} src/rendering/drawsimpleobject.h src/rendering/drawobject.h src/rendering/rendertext.h src/rendering/distanceanglediedre.h )
  set( in_MOC_H ${in_MOC_H} src/avo_toolbox/moleculemanipulation.h src/avo_toolbox/settingswidget.h )
  #set( in_MOC_H ${in_MOC_H} src/avo_toolbox/undocommand.h src/avo_toolbox/constraintsmodel.h )
  set( in_MOC_H ${in_MOC_H} src/inputdevice/inputdevice.h src/inputdevice/wmdevice.h src/inputdevice/wiimote/wmrumble.h )
  #set( in_MOC_H ${in_MOC_H} src/inputdevice/wiimote/wmavo.h ) # Add file to include in project, but it does not need moc file ...
  set( in_MOC_H ${in_MOC_H} src/wrapper/wrapper.h src/wrapper/chemicalwrapper.h src/wrapper/chemical/wrapper_chemicalcmd_to_avoaction.h src/wrapper/chemical/contextmenu_to_avoaction.h )
  set( in_MOC_H ${in_MOC_H} src/qt_extended/qmenu_ex.h src/qt_extended/qaction_ex.h src/qt_extended/qundo_ex.h )
  
  set( in_UIS_H "" )
  set( in_RC_SCRS src/inputdevice/wiimote/wmtool.qrc )
  
  # All source file (recursif search) (wmavo, fifo ...).
  # Add the forgotten files for the moc generation.
  file( GLOB_RECURSE source_files	src/* )
  
  # Add flags for Windows.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") # Add a flag in a CMake attribut.

  
  ## Set up the build environment (to know where are .h, ".dll" and some files)

  # Let to use .h directly : 
  # ex. : #include <wmavo_const.h> instead of <../../wmavo_const.h>
  include_directories( ${wmavoconst_include_dir} )
  include_directories( ${tool_include_dir} )
  include_directories( ${avo_toolbox_include_dir} )
  include_directories( ${inputdevice_include_dir} )
  include_directories( ${qt_extended_include_dir} )
  include_directories( ${rendering_include_dir} )
  include_directories( ${toolbox_include_dir} )
  include_directories( ${wrapper_include_dir} )
  include_directories( ${wiiuse_include_dir} )
  include_directories( ${wiiusecpp_include_dir} )

  include_directories( ${OPENBABEL2_INCLUDE_DIR} )
  
  # ".dll" directories.
  link_directories( ${wiiuse_libd_dir} )
  
  # Other files.
  include(${Avogadro_USE_FILE})

  ## BELOW, NO CHANGE ! 
  # (Except to add a target_link_libraries.)

  ## Génération des fichiers nécessaire pour la compilation 
  ## et éditions de liens avec Qt, Avogadro & Co.
  QT4_WRAP_CPP(out_MOC_H ${in_MOC_H})
  qt4_wrap_ui(out_UIS_H ${in_UIS_H})
  qt4_add_resources(out_RC_SRCS ${in_RC_SCRS})

  add_library( ${PLUGIN_NAME} MODULE ${source_files} ${out_UIS_H} ${out_MOC_H} ${out_RC_SRCS} )
  
  # Link the LIBRARY ! Not *.h !
  target_link_libraries( ${PLUGIN_NAME} ${wiiuse_lib_name} ) 
  target_link_libraries( ${PLUGIN_NAME} ${wiiusecpp_lib_name} )
  target_link_libraries( ${PLUGIN_NAME} avogadro )
  target_link_libraries( ${PLUGIN_NAME} ${OPENGL_LIBRARIES} )
  
  # ?
  #set_target_properties(${PLUGIN_NAME} PROPERTIES
  #                      OUTPUT_NAME ${PLUGIN_NAME}
  #                      PREFIX ""
  #                      LABELS "${PLUGIN_LABEL}")
  #set_target_properties(${PLUGIN_NAME} PROPERTIES
  #                      COMPILE_FLAGS "${NO_RTTI_FLAG}")
  
endif()