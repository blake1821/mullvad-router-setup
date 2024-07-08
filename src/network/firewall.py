from ipaddress import IPv4Address
from typing import Optional
from data.network import PortForward
from data.network import IfName
from util.bash import bash

def forward_port(
    pf: PortForward,
    incoming_interface: IfName
):
    bash(f'''
            iptables -t nat -A PREROUTING -i {incoming_interface} -p tcp \
            --dport {pf.src_port} -j DNAT \
            --to-destination {pf.dst_addr}:{pf.dst_port}
        ''')

def set_vpn_firewall(
        *,
        wg: IfName,
        lan: IfName,
        man: IfName,
        wan: IfName,
        peer_ip: Optional[IPv4Address]
):
    if not peer_ip:
        peer_ip = IPv4Address('0.0.0.0')

    bash(f'''
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

        # Forward traffic from lan_ifname to wg_ifname for IPv4 and IPv6
        iptables -A FORWARD -i {lan} -o {wg} -j ACCEPT
        ip6tables -A FORWARD -i {lan} -o {wg} -j ACCEPT
        iptables -A FORWARD -i {wg} -m state --state ESTABLISHED,RELATED -j ACCEPT
        ip6tables -A FORWARD -i {wg} -m state --state ESTABLISHED,RELATED -j ACCEPT

        # Block all incoming connections on lan_ifname except for DHCP for IPv4 and IPv6
        iptables -A INPUT -i {lan} -p udp --dport 67:68 --sport 67:68 -j ACCEPT
        ip6tables -A INPUT -i {lan} -p udp --dport 546:547 --sport 546:547 -j ACCEPT
        ip6tables -A INPUT -i {lan} -p icmpv6 -j ACCEPT

        # Allow all incoming connections on man_ifname for IPv4 and IPv6
        iptables -A INPUT -i {man} -j ACCEPT
        ip6tables -A INPUT -i {man} -j ACCEPT

        # Allow established connections to input
        iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
        ip6tables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

        iptables -t nat -A POSTROUTING -o {wg} -j MASQUERADE
        ip6tables -t nat -A POSTROUTING -o {wg} -j MASQUERADE

        iptables -A FORWARD -i {man} -o {lan} -j ACCEPT
        iptables -A FORWARD -i {lan} -o {man} -m state --state ESTABLISHED,RELATED -j ACCEPT

        # Block outgoing connections on WAN except those to the peer
        iptables -A OUTPUT -o {wan} -d {peer_ip} -j ACCEPT
        iptables -A OUTPUT -o {wan} -j DROP
        ip6tables -A OUTPUT -o {wan} -j DROP
    ''')

def set_basic_v4_firewall():
    bash('''
        iptables -F
        iptables -X
        iptables -P OUTPUT ACCEPT
        iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
''')