from ipaddress import IPv4Address, IPv4Interface, IPv6Address, IPv6Interface
from data.network import IfName
from util import bash, bash_get

def reset_wg_interface(wg: IfName):
    bash(f'ip link del {wg} 2>/dev/null')
    bash(f'ip link add dev {wg} type wireguard')


def add_interface_ip(*, ifname: IfName, ipv4: IPv4Address, ipv6: IPv6Address):
    bash(f'ip addr add dev {ifname} {ipv4}')
    bash(f'ip -6 addr add dev {ifname} {ipv6}')

def add_route_to_peer(peer_ip: IPv4Address):
    default_gateway = bash_get("ip route show | grep 'default via' | awk '{print $3}'")
    bash(f'ip route add {peer_ip} via {default_gateway}')

def bring_interface_up(interface: IfName):
    bash(f'ip link set up dev {interface}')

def route_traffic_through_interface(interface: IfName):
    bash(f'''
        ip route add 0.0.0.0/1 dev {interface}
        ip route add 128.0.0.0/1 dev {interface}
        ip -6 route delete default
        ip -6 route add default dev {interface}
    ''')

def remove_reroute():
    bash(f'''
        ip route del 0.0.0.0/1
        ip route del 128.0.0.0/1
    ''')


def set_forwarding_enabled(enabled: bool):
    bash(f'''
        sysctl -w net.ipv4.ip_forward={1 if enabled else 0} 1>/dev/null
        sysctl -w net.ipv6.conf.all.forwarding={1 if enabled else 0} 1>/dev/null
    ''')


class IPHelper:
    def __init__(self, *,
                 ipv4_interface: IPv4Interface | None = None,
                 ipv6_interface: IPv6Interface | None = None):
        if ipv4_interface:
            self.interface = ipv4_interface
        elif ipv6_interface:
            self.interface = ipv6_interface
        else:
            raise ValueError("Expected either ipv4_interface or ipv6_interface")

    def get_first_client_ip(self):
        return self.interface.ip + 1

    def get_last_client_ip(self):
        return self.interface.network.broadcast_address - 1

    def get_ip(self):
        return self.interface.ip

    def get_network_address(self):
        return self.interface.network.network_address

    def get_netmask(self):
        return self.interface.network.netmask

    def get_network(self):
        return self.interface.network