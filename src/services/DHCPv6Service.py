from config.config_access import ConfigDAO
from services.service import Service
from network.ip import IPHelper

class DHCPv6Service(Service):
    def __init__(self) :
        super().__init__(
            apt_packages=[],
            service_name=None,
            config_path='/etc/dhcp/dhcpd6.conf'
        )

    def get_config_string(self, config: ConfigDAO) -> str:
        lan6 = IPHelper(ipv6_interface=config.lan_v6_interface)
        return f'''
default-lease-time 600;
max-lease-time 7200;

subnet6 {lan6.get_network()} {{
    range6 {lan6.get_first_client_ip()} {lan6.get_last_client_ip()};
    option dhcp6.name-servers {config.dns6};
}}
'''