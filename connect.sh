#!/bin/bash

VPN_HOME=`dirname $0`
VPN_HOME=`realpath $VPN_HOME`
cd $VPN_HOME

# get variables from setup.sh
cd const
LAN_IFNAME=$(cat lan)
WAN_IFNAME=$(cat wan)
MAN_IFNAME=$(cat man)
DNS_SERVER=$(cat dns)
DNS6_SERVER=$(cat dns6)
cd ..

# constants
WG_IFNAME=wg0
PORT=51820

ip link del $WG_IFNAME 2>/dev/null
ip link add dev $WG_IFNAME type wireguard

# Default gateway IP
DEFAULT_GATEWAY=$(ip route show | grep "default via" | awk '{print $3}')

if [[ -f endpoint/relay_ip_pubkeys ]]; then 
    cd endpoint
    MY_IPV4_ADDRESS=$(cat my4)
    MY_IPV6_ADDRESS=$(cat my6)

    # Select a random endpoint IP
    ENDPOINT_IP_PUBKEY=`shuf relay_ip_pubkeys | head -1`
    ENDPOINT_IP=`echo "$ENDPOINT_IP_PUBKEY" | cut -f1`
    ENDPOINT_PUBKEY=`echo "$ENDPOINT_IP_PUBKEY" | cut -f2`

    # Set up the interface
    ip addr add dev $WG_IFNAME $MY_IPV4_ADDRESS
    ip -6 addr add dev $WG_IFNAME $MY_IPV6_ADDRESS
    wg set $WG_IFNAME \
        listen-port $PORT \
        private-key ../const/privatekey \
        peer $ENDPOINT_PUBKEY \
            endpoint $ENDPOINT_IP:$PORT \
            allowed-ips 0.0.0.0/0,::/0

    # Make sure we can route to the endpoint through the default gateway
    ip route add $ENDPOINT_IP via $DEFAULT_GATEWAY
    cd ..
fi

ip link set up dev $WG_IFNAME

# Route all traffic through the wireguard tunnel
ip route add 0.0.0.0/1 dev $WG_IFNAME
ip route add 128.0.0.0/1 dev $WG_IFNAME
ip -6 route delete default
ip -6 route add default dev $WG_IFNAME

# Enable forwarding
sysctl -w net.ipv4.ip_forward=1
sysctl -w net.ipv6.conf.all.forwarding=1

# Set up firewall rules
    # Flush existing rules and set default chain policies for IPv4
    iptables -F
    iptables -X
    iptables -P INPUT DROP
    iptables -P FORWARD DROP
    iptables -P OUTPUT ACCEPT

    # Flush existing rules and set default chain policies for IPv6
    ip6tables -F
    ip6tables -X
    ip6tables -P INPUT DROP
    ip6tables -P FORWARD DROP
    ip6tables -P OUTPUT ACCEPT

    # Allow loopback interface to function properly for IPv4 and IPv6
    iptables -A INPUT -i lo -j ACCEPT
    iptables -A OUTPUT -o lo -j ACCEPT
    ip6tables -A INPUT -i lo -j ACCEPT
    ip6tables -A OUTPUT -o lo -j ACCEPT

    # Forward traffic from $LAN_IFNAME to $WG_IFNAME for IPv4 and IPv6
    iptables -A FORWARD -i $LAN_IFNAME -o $WG_IFNAME -j ACCEPT
    ip6tables -A FORWARD -i $LAN_IFNAME -o $WG_IFNAME -j ACCEPT
    iptables -A FORWARD -i $WG_IFNAME -m state --state ESTABLISHED,RELATED -j ACCEPT
    ip6tables -A FORWARD -i $WG_IFNAME -m state --state ESTABLISHED,RELATED -j ACCEPT

    # Block all incoming connections on $LAN_IFNAME except for DHCP for IPv4 and IPv6
    iptables -A INPUT -i $LAN_IFNAME -p udp --dport 67:68 --sport 67:68 -j ACCEPT
    ip6tables -A INPUT -i $LAN_IFNAME -p udp --dport 546:547 --sport 546:547 -j ACCEPT

    # Allow all incoming connections on $MAN_IFNAME for IPv4 and IPv6
    iptables -A INPUT -i $MAN_IFNAME -j ACCEPT
    ip6tables -A INPUT -i $MAN_IFNAME -j ACCEPT

    # Allow established connections to input
    iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
    ip6tables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

    iptables -t nat -A POSTROUTING -o $WG_IFNAME -j MASQUERADE
    ip6tables -t nat -A POSTROUTING -o $WG_IFNAME -j MASQUERADE

    # Port forwarding
    shopt -s nullglob
    mkdir port-fw 2>/dev/null
    cd port-fw
    for src_port in *
    do
        iptables -t nat -A PREROUTING -i $MAN_IFNAME -p tcp \
                --dport $src_port -j DNAT \
                --to-destination $(head -n 1 $src_port):$(tail -n 1 $src_port)
    done
    cd ..
    iptables -A FORWARD -i $MAN_IFNAME -o $LAN_IFNAME -j ACCEPT
    iptables -A FORWARD -i $LAN_IFNAME -o $MAN_IFNAME -m state --state ESTABLISHED,RELATED -j ACCEPT

    # Block outgoing connections on WAN except those to the peer
    iptables -A OUTPUT -o $WAN_IFNAME -d $ENDPOINT_IP -j ACCEPT
    iptables -A OUTPUT -o $WAN_IFNAME -j DROP
    ip6tables -A OUTPUT -o $WAN_IFNAME -j DROP


# Setup DNS
cat >/etc/resolv.conf <<EOF
nameserver $DNS_SERVER
nameserver $DNS6_SERVER
EOF

# Restart dante
systemctl restart danted
