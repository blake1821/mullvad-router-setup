from dataclasses import dataclass
from ipaddress import IPv4Address
from typing import Iterable
from config.data_access import ConfigAccessor
from services.service import Service
from network.ip import IPHelper
from util import bash_get

@dataclass
class DHCPClient:
    mac_address: str
    ip_address: IPv4Address
    hostname: str

class DHCPService(Service):
    def __init__(self) :
        super().__init__(
            apt_packages=['isc-dhcp-server'],
            service_name='isc-dhcp-server',
            config_path='/etc/dhcp/dhcpd.conf'
        )

    def get_config_string(self, config: ConfigAccessor) -> str:
        lan4 = IPHelper(ipv4_interface=config.lan_v4_interface)
        return f'''
option domain-name-servers {config.dns};

default-lease-time 600;
max-lease-time 7200;
ddns-update-style none;

subnet {lan4.get_network_address()} netmask {lan4.get_netmask()} {{
    range {lan4.get_first_client_ip()} {lan4.get_last_client_ip()};
    option routers {lan4.get_ip()};

    interface {config.lan_ifname};
}}
'''

    def get_clients(self) -> Iterable[DHCPClient]:
        lines = bash_get("dhcp-lease-list --parsable 2>/dev/null").splitlines()
        return (
            DHCPClient(
                mac_address = entries[1],
                ip_address=IPv4Address(entries[3]),
                hostname=entries[5]
            )
            for line in lines
            for entries in [line.split(' ')]
        )