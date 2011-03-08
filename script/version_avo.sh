version_avo=`avogadro --version | head -1 | cut -d " " -f 2 | sed -re 's/\t//g'`
echo ""
echo "Test:$version_avo:"

test="test"
if [ $test == "test" ] ; then
  echo "OK"
else
  echo "PAS OK"
fi

echo "Appuyé sur y"
read a

if [ $a == "y" ] ; then
  echo 'Y'
else
  echo 'N'
fi
