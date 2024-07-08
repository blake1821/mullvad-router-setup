import time
from typing import Literal
from config.config_access import ReloadHandler
from services.CustomService import CustomService
from services.DHCPService import DHCPService
from services.DHCPv6Service import DHCPv6Service
from services.InterfacesService import InterfacesService
from services.NameService import NameService
from services.ProxyService import ProxyService
from services.RAService import RAService
from services.service import Service
from util.util import get_vpn_home

vpn_home = get_vpn_home()

PROXY_SERVICE = ProxyService()
NAME_SERVICE = NameService()
MULLVAD_SERVICE = CustomService(
    name='mullvad',
    description='Connect to a VPN',
    exec_path=f'{vpn_home}/connect.sh',
    restart_mode='no'
)
ROUTER_WEBSITE_SERVICE = CustomService(
    name='router-website',
    description='mullvad-router website',
    exec_path=f'{vpn_home}/website.sh',
    restart_mode='always'
)
DHCP_SERVICE = DHCPService()

SERVICES: list[Service] = [
    # Common utils
    Service(apt_packages=['wireguard', 'jq', 'curl', 'iptables', 'sshpass']),

    InterfacesService(),
    DHCP_SERVICE,
    DHCPv6Service(),
    RAService(),
    PROXY_SERVICE,
    NAME_SERVICE,
    MULLVAD_SERVICE,
    ROUTER_WEBSITE_SERVICE
]

class MullvadReloadHandler(ReloadHandler):
    def reload(self, reload: Literal['reload']) -> None:
        MULLVAD_SERVICE.restart()
        time.sleep(1)