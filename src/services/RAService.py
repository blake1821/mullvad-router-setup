from config.config_access import ConfigDAO
from services.service import Service
from network.ip import IPHelper


class RAService(Service):
    def __init__(self):
        super().__init__(
            apt_packages=['radvd'],
            service_name='radvd',
            config_path='/etc/radvd.conf'
        )

    def get_config_string(self, config: ConfigDAO) -> str:
        lan6 = IPHelper(ipv6_interface=config.lan_v6_interface)
        return f'''
interface {config.lan_ifname} {{

    AdvSendAdvert on;
    MaxRtrAdvInterval 10;

    prefix {lan6.get_network()} {{
        AdvOnLink on;
        AdvAutonomous on;
    }};

    RDNSS {config.dns6} {{
        AdvRDNSSLifetime 20;
    }};
}};
'''