#!/bin/bash

TYPE=$1
ID=$2

echo "$TYPE
$ID">endpoint

systemctl restart mullvad 2>&1
sleep 1
echo redirect
