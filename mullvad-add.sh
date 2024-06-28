#!/bin/bash

LOCATION=$1
MY_IPV4_ADDRESS=$2
MY_IPV6_ADDRESS=$3

# Temporarily clear existing routes & rules,
# so that we can connect to Mullvad's API
systemctl stop danted
sysctl -w net.ipv4.ip_forward=0 1>/dev/null
sysctl -w net.ipv6.conf.all.forwarding=0 1>/dev/null
ip route del 0.0.0.0/1
ip route del 128.0.0.0/1
iptables -F
iptables -X
iptables -P OUTPUT ACCEPT
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

MULLVAD_ID=$(cat /dev/urandom | head -c 20 | md5sum | head -c 20)

mkdir mullvad 2>/dev/null
cd mullvad
exec 3>$MULLVAD_ID
echo $LOCATION >&3
echo $MY_IPV4_ADDRESS >&3
echo $MY_IPV6_ADDRESS >&3
RELAYS=$(curl https://api.mullvad.net/app/v1/relays)
paste \
    <(echo $RELAYS | jq -r ".wireguard.relays[] | select(.location | startswith(\"$LOCATION\")) | .ipv4_addr_in") \
    <(echo $RELAYS | jq -r ".wireguard.relays[] | select(.location | startswith(\"$LOCATION\")) | .public_key") \
    $RELAY_PUBKEYS >&3
exec 3<&-
cd ..

systemctl restart mullvad 2>&1
echo redirect
