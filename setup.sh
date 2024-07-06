#!/bin/bash

VPN_HOME=`dirname $0`
VPN_HOME=`realpath $VPN_HOME`
cd $VPN_HOME
chmod a+x *.sh *.py

python3 src/setup.py