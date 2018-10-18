#!/bin/bash

EXEC=/tmp/make_amalgamation.bash
CPP=amalgamation.cpp

rm $CPP

echo "#!/bin/bash
echo \$1
if [ \"x\" = \"x\$1\" ] ; then
	exit
fi

cat \$1 >> $CPP
" > $EXEC

chmod u+x $EXEC

cat $EXEC

find -type f -name "*.cpp" -exec $EXEC {} \;

rm $EXEC
