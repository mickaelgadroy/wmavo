#!/bin/bash

# Execution :
# Either :  $ bash 2_compile_install.sh


# Information about dependency.
echo "Verify the dependency with (1_dependency.sh) script"
echo "before to continue the plugin compilation."
echo "(Here the user account is enough.)"
echo "(Press any button to continue, or press Ctl+C to abort.)"
read

which avogadro
if [ $? -ne 0 ] ; then
  echo "error: Avogadro seems not to exist ..."
  echo "error: Instalation aborted."
  exit 1
fi

version_avo=`avogadro --version | head -1 | cut -d " " -f 2 | sed -re 's/\t//g'`

echo "Avogadro version:$version_avo"
version_avo_major=`echo $version_avo | cut -d "." -f 1`
version_avo_minor=`echo $version_avo | cut -d "." -f 2`
version_avo_patch=`echo $version_avo | cut -d "." -f 3`


echo ""
echo "info: Part II: Compilation and installation of wiimote plugins"
echo ""

cd ../src/
build_dir="build_auto"

for dir in {en,ex,tool} ; do

  echo ""
  echo "info: Compiling src/$dir directory"

  # if source directory noexisting ?
  if [ ! -d $dir ] ; then
    echo "error: The directory $dir is not present !"
    echo "error: Instalation aborted."
    exit 1
  fi

  cd $dir

  # if build directory existing ?
  if [ -d $build_dir ] ; then
    echo ""
    echo "info: Clean up src/$dir/$build_dir directory"
    rm -r $build_dir
  fi

  echo ""
  echo "info: Create src/$dir/$build_dir directory"
  mkdir $build_dir
  cd $build_dir

  echo ""  
  echo "info: Cmake ..."
  cmake -G"Unix Makefiles" ../
  #if cmake error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during cmake for src/$dir/$dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  echo ""
  echo "info: Make ..."
  #sudo make clean
  make
  #if make error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  if [ $dir == "en" ] ; then
    dir_type="engines"
  fi
  if [ $dir == "ex" ] ; then
    dir_type="extensions"  
  fi
  if [ $dir == "tool" ] ; then
    dir_type="tools"
  fi
    
  ## Copy (1st version) the lib in Avogadro directory.
  # Be careful with the version.
  #if [[ $version_avo == "1.0.1" || $version_avo == "1.0.0" ]] ; then
  #  dir_lib="/usr/lib/avogadro/1_0/$dir_type/"
  #else
  #  dir_lib="/usr/local/lib/avogadro/$version_avo_major""_$version_avo_minor/$dir_type/"
  #fi
  
  ## Copy (2nd version) the lib in user directory.
  dir_lib="/home/`whoami`/.avogadro"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi
  
  dir_lib="$dir_lib/$version_avo_major""_$version_avo_minor"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi
  
  dir_lib="$dir_lib/plugins"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi

  dir_lib="$dir_lib/extensions/"
  if [ ! -d $dir_lib ] ; then
    mkdir $dir_lib
  fi
  
  ok=0
  while [ $ok -eq 0 ] ; do
  
    echo ""
    echo "info: The $dir_type plugin will be copied in:"
    echo $dir_lib
    echo "info: This repertory seems ok ? (y/n) (y:default value)"
    
    read a
    
    if [ "$a" != "" ] ; then
      # You don't just pressed Enter.
      
      if [ $a != "y" ] ; then
        echo "info: Enter your desired path:"
        read dir_lib
      fi
    fi
    
    cp libwm*.so $dir_lib
    #if cp error ...
    if [ $? -ne 0 ] ; then
      echo "error: Directory $dir_lib causes problem during copy"
    else
      ok=1
    fi
  done
  
  cd ../../

done

cd ../script
exit 0
