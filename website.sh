#!/bin/bash

VPN_HOME=`dirname $0`
VPN_HOME=`realpath $VPN_HOME`
cd $VPN_HOME

python3 src/router-website-service.py