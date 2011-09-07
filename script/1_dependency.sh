#!/bin/bash

# Execution :
# Either :  $ bash 1_dependency.sh


# Information about dependency.
echo "Verify if you have this dependency:"
echo "apt-get install gcc g++ make cmake"
echo "apt-get install zlib1g zlib1g-dev (1.2.3 at least)"
echo "apt-get install python-dev libboost-python-dev (respectively : 2.6 et 1.40 at least)"
echo "apt-get install glutg3 glutg3-dev libglew1.5 libglew1.5-dev"
echo "apt-get install avogadro libavogadro-dev (1.0.0 at least)"
echo "apt-get install bluez libbluetooth-dev (4.60 at least)"
echo ""
echo "Be sur to install the previous package (you must to be root)"
echo "(Press any button to continue, or press Ctl+C to abort.)"
read


#if wiiuse/wiiusecpp libraries existing
# search the files ...
if [ -r /usr/lib/libwiiuse.so ] ; then
  echo "info: libwiiuse.so existing and accessible in /usr/lib/"
fi

if [ -r /usr/lib/libwiiusecpp.so ] ; then
  echo "info: libwiiusecpp.so existing and accessible in /usr/lib/"
fi

if [ ! -d ../extra/ ] ; then
  echo "error: The directory ../extra/ is not present !"
  echo "error: Instalation aborted."
  exit 1
fi

cd ../extra/

if [ ! -d ../extra/wiiuse_wiiusecpp ] ; then
  echo "error: The directory ../extra/wiiuse_wiiusecpp is not present !"
  echo "error: Instalation aborted."
  exit 1
fi

cd wiiuse_wiiusecpp/

echo ""
echo "info: Part I : Compilation and installation of wiiuse/wiiusecpp librairies (even if existing)"
echo ""

for dir in {wiiuse,wiiusecpp} ; do

  echo ""
  echo "info: In extra/wiiuse_wiiusecpp/$dir directory"

  # if source directory noexisting ?
  if [ ! -d $dir ] ; then
    echo "error: The directory $dir is not present !"
    echo "error: Instalation aborted."
    exit 1
  fi

  cd $dir/src/

  echo ""
  echo "info: Clean up $dir before the compilation"
  make distclean
  
  echo ""
  echo "info: Compilation of $dir"
  #make $dir
  make
  #if make error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  echo ""
  echo "info: Installation (just copy) of $dir in : /usr/lib/lib$dir.so"
  sudo make install
  #if make install error ...
  if [ $? -ne 0 ] ; then
    echo "error: An error is appeared during make install for $dir."
    echo "error: Instalation aborted."
    exit 1
  fi

  cd ../../

done

cd ../../script

exit 0

