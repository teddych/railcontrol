#!/bin/bash

EXEC=./make_amalgamation.bash
CPP=amalgamation.cpp

rm -f $CPP

echo "#!/bin/bash
echo Preparing \$1
if [ \"x\" = \"x\$1\" ] ; then
	exit
fi

cat \$1 >> $CPP
" > $EXEC

chmod u+x $EXEC

for dir in . DataModel Hardware Hardware/Protocols Logger Network Storage Utils Server/CS2 Server/Web Server/Z21 ; do
	find $dir -maxdepth 1 -type f -name "*.cpp" -exec $EXEC {} \;
done

rm $EXEC
