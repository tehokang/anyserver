#!/bin/bash

HAS_DOXYGEN=`which doxygen`
HAS_GRAPHVIZ=`which graphml2gv`

if [ -z "HAS_DOXYGEN" ]; then
	echo "You have to install doxygen"
	echo "   e.g.) sudo apt-get install doxygen"
	exit 1
fi

VER_DOXYGEN=`doxygen --version`
echo "Your system already has doxygen (" $VER_DOXYGEN ") and graphviz"

function check_interrupt_by_signal()
{
	echo "========================================"
	echo " You stopped the documenting of doxygen "
	echo "========================================"
	exit 1
}

trap check_interrupt_by_signal INT

doxygen $PWD/doxygen-anyserver

echo "========================================================================"
echo "    Please refer to output/html directory which has doxygen for web"
if [ -z "HAS_GRAPHVIZ" ]; then
	echo " If you want to include class diagram, You have to install graphviz "
	echo "   e.g.) sudo apt-get install graphviz"
fi
echo "========================================================================"
