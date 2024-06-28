#!/bin/bash

# Web page script

src_port=$1
dst_addr=$2
dst_port=$3

mkdir port-fw

if [[ $dst_addr == "delete" ]]; then
rm port-fw/$src_port
else
cat >port-fw/$src_port <<EOF
$dst_addr
$dst_port
EOF
fi

systemctl restart mullvad 2>&1
sleep 1
echo 'redirect'
