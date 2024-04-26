#!/bin/bash

VPN_HOME=~
cd $VPN_HOME

echo VPN Router Setup
echo ==============================
echo

# Install packages
apt install wireguard jq curl iptables isc-dhcp-server dante-server

# Prompt for information
echo Here are the different interfaces: 
ip link show | awk -F': ' '/^[0-9]/ {print $2}'
echo
read -p "Enter the WAN interface name: " WAN_IFNAME
read -p "Enter the MAN interface name: " MAN_IFNAME
read -p "Enter the LAN interface name: " LAN_IFNAME
echo
echo Go to mullvad.net and create a new device.
echo Generating Private Key...
PRIVATE_KEY=$(wg genkey)
PUBLIC_KEY=$(echo $PRIVATE_KEY | wg pubkey)
echo The public key is $PUBLIC_KEY
read -p "Enter the IPV4 From Mullvad: " MY_IPV4_ADDRESS
read -p "Enter the IPV6 From Mullvad: " MY_IPV6_ADDRESS


DNS_SERVER=1.1.1.1
DNS6_SERVER=2606:4700:4700::1111

# Write Variables
echo $PRIVATE_KEY >privatekey
echo $PUBLICK_KEY >publickey
mkdir vars
cd vars
echo $LAN_IFNAME>lan
echo $WAN_IFNAME>wan
echo $MAN_IFNAME>man
echo $DNS_SERVER>dns
echo $DNS6_SERVER>dns6
echo $MY_IPV4_ADDRESS>my4
echo $MY_IPV6_ADDRESS>my6
cd ..

# /etc/network/interfaces
cat >/etc/network/interfaces <<EOF
source /etc/network/interfaces.d/*

auto lo
iface lo inet loopback

auto $WAN_IFNAME
allow-hotplug $WAN_IFNAME
iface $WAN_IFNAME inet dhcp

auto $MAN_IFNAME
allow-hotplug $MAN_IFNAME
iface $MAN_IFNAME inet dhcp

auto $LAN_IFNAME
iface $LAN_IFNAME inet static
        address 192.168.0.1/24
iface $LAN_IFNAME inet6 static
        address 2001:db8::1/64
EOF

# /etc/dhcp
# dhcpd.conf
cat >/etc/dhcp/dhcpd.conf <<EOF
option domain-name-servers $DNS_SERVER;

default-lease-time 600;
max-lease-time 7200;
ddns-update-style none;

subnet 192.168.0.0 netmask 255.255.255.0 {
    range 192.168.0.2 192.168.0.254;
    option routers 192.168.0.1;

    interface $LAN_IFNAME;
}
EOF

# dhcpd6.conf
cat >/etc/dhcp/dhcpd6.conf <<EOF

default-lease-time 2592000;
preferred-lifetime 604800;
option dhcp-renewal-time 3600;
option dhcp-rebinding-time 7200;
allow leasequery;

option dhcp6.name-servers $DNS6_SERVER;

subnet6 2001:db8::/64 {
    range6 2001:db8::100 2001:db8::1:0;
    interface $LAN_IFNAME;
}

EOF

# /etc/danted.conf
cat >/etc/danted.conf <<EOF
logoutput: syslog
user.privileged: root
user.unprivileged: nobody

# The listening network interface or address.
internal: $MAN_IFNAME port=1080

# The proxying network interface or address.
external: wg0

# socks-rules determine what is proxied through the external interface.
clientmethod: none
socksmethod: none

client pass {
    from: 0/0 to: 0/0
}

socks pass {
    from: 0/0 to: 0/0
}
EOF
