#!/bin/bash

TYPE=$1
MY_IPV4_ADDRESS=$2
MY_IPV6_ADDRESS=$3

# Temporarily clear existing routes & rules
systemctl stop danted
sysctl -w net.ipv4.ip_forward=0 1>/dev/null
sysctl -w net.ipv6.conf.all.forwarding=0 1>/dev/null
ip route del 0.0.0.0/1
ip route del 128.0.0.0/1
iptables -F
iptables -X
iptables -P OUTPUT ACCEPT
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

mkdir endpoint 2>/dev/null
cd endpoint
echo $MY_IPV4_ADDRESS>my4
echo $MY_IPV6_ADDRESS>my6
if [[ $TYPE == 'mullvad' ]]; then
    LOCATION=$4
    curl https://api.mullvad.net/app/v1/relays > relays

    RELAY_IPS=$(mktemp);
    RELAY_PUBKEYS=$(mktemp);

    cat relays  \
        | jq -r ".wireguard.relays[] | select(.location | startswith(\"$LOCATION\")) | .ipv4_addr_in" \
        >$RELAY_IPS
    cat relays  \
        | jq -r ".wireguard.relays[] | select(.location | startswith(\"$LOCATION\")) | .public_key" \
        >$RELAY_PUBKEYS

    paste $RELAY_IPS $RELAY_PUBKEYS >relay_ip_pubkeys

    rm $RELAY_IPS
    rm $RELAY_PUBKEYS
    # rm relays
else
    ENDPOINT_IP=$4
    ENDPOINT_PUBKEY=$5
    echo "$ENDPOINT_IP $ENDPOINT_PUBKEY" >relay_ip_pubkeys
fi
cd ..

systemctl restart mullvad 2>&1
sleep 1
echo redirect
