#!/bin/bash

# Execution :
# Either : 
#      $ bash install_test.sh



#if library existing
# search the files ...
if [ -r /usr/lib/libwiiuse.so ] ; then
  echo "info: libwiiuse existing and accessible"
fi

if [ -r /usr/lib/libwiiusecpp.so ] ; then
  echo "info: libwiiusecpp existing and accessible"
fi

cd ../extra/


echo ""
echo "Part I : Compilation and installation of wiiuse/wiiusecpp"

for dir in {wiiuse,wiiusecpp} ; do

  echo "info: Compiling extra/$dir directory"

  # if source directory noexisting ?
  if [ ! -d $dir ] ; then
    echo "error: The directory $dir is not present !"
    echo "error: Instalation aborted."
    exit 1
  fi

  cd $dir

  echo ""
  echo "Clean up $dir before the compilation"
  make distclean
  
  echo ""
  echo "Compilation of $dir"
  sudo make $dir
  #if make error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  echo ""
  echo "Installation of $dir in /usr/lib/lib$dir.so"
  sudo make install
  #if make install error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make install for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  cd ../

done

cd ../src


echo ""
echo "Part II: Compilation and installation of wiimote plugins"

for dir in {en,ex,tool} ; do

  echo "info: Compiling src/$dir directory"

  # if source directory noexisting ?
  if [ ! -d $dir ] ; then
    echo "error: The directory $dir is not present !"
    echo "error: Instalation aborted."
    exit 1
  fi

  cd $dir

  # if build directory existing ?
  if [ -d build ] ; then
    sudo rm -r build
  fi

  mkdir build
  cd build
  
  cmake -G"Unix Makefiles" ../
  #if cmake error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during cmake for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  #sudo make clean
  sudo make
  #if make error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  version_avo=`avogadro --version | head -1 | cut -d " " -f 2 | sed -re 's/\t//g'`
  echo "Avogadro version:$version_avo"
  
  if [ $dir == "en" ] ; then
    dir_type="engines"
  fi
  if [ $dir == "ex" ] ; then
    dir_type="extensions"  
  fi
  if [ $dir == "tool" ] ; then
    dir_type="tools"
  fi
    
  # Be careful with the version.
  if [[ $version_avo == "1.0.1" || $version_avo == "1.0.0" ]] ; then
    dir_lib="/usr/lib/avogadro/1_0/$dir_type/"
  else
    dir_lib="/usr/local/lib/avogadro/1_0/$dir_type/"
  fi
  
  ok=0
  while [ $ok -eq 0 ] ; do
  
    echo ""
    echo "The $dir_type plugin will be copied in:"
    echo $dir_lib
    echo "This repertory seems ok ? (y/n) (y:default value)"
    
    read a
    if [ $a != "y" ] ; then
      echo "Enter your desired path:"
      read dir_lib
    fi
    
    sudo cp libwm*.so $dir_lib
    
    if [ $? -ne 0 ] ; then
      echo "error: Directory $dir_lib causes problem."
    else
      ok=1
    fi
  done
  
  cd ../../

done

