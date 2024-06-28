#!/bin/bash

MULLVAD_ID=$1
cd mullvad
rm $MULLVAD_ID
cd ..

systemctl restart mullvad 2>&1
echo redirect
