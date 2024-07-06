from config.config_access import ConfigAccessor
from services.service import Service

class InterfacesService(Service):
    def __init__(self) :
        super().__init__(
            apt_packages=[],
            service_name='networking',
            config_path='/etc/network/interfaces'
        )

    def get_config_string(self, config: ConfigAccessor) -> str:
        return f'''\
source /etc/network/interfaces.d/*

auto lo
iface lo inet loopback

auto {config.wan_ifname}
allow-hotplug {config.wan_ifname}
iface {config.wan_ifname} inet dhcp

auto {config.man_ifname}
allow-hotplug {config.man_ifname}
iface {config.man_ifname} inet dhcp

auto {config.lan_ifname}
iface {config.lan_ifname} inet static
        address {config.lan_v4_interface}
iface {config.lan_ifname} inet6 static
        address {config.lan_v6_interface}
'''