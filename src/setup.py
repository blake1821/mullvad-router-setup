from config.config import Config, ConfigManager, get_setup_config
from data.network import IfName, PrivateKey
from config.config_access import ConfigAccessor
from services.services import SERVICES, MullvadReloadHandler
from util import bash, get_random_string

print('''\
VPN Router Setup
=====================================\
''')

setup_config = get_setup_config()

# we removed dnsmasq. not sure why it was installed
bash(f'apt install {" ".join((package for service in SERVICES for package in service.apt_packages))}')
print('Here are the different interfaces:')
bash("ip link show | awk -F': ' '/^[0-9]/ {print $2}'")
print()

wan_ifname = IfName(input("Enter the WAN interface name: "))
man_ifname = IfName(input("Enter the MAN interface name: "))
lan_ifname = IfName(input("Enter the LAN interface name: "))
print()
router_password = input("Enter a password for the router website: ")
print()

config_manager = ConfigManager()
config_manager.save(Config({
    "wan_ifname": wan_ifname,
    "man_ifname": man_ifname,
    "lan_ifname": lan_ifname,
    "router_password": router_password,
    "private_key": PrivateKey.generate(),
    "login_cookie": get_random_string(),
    "mullvad_devices": {},
    "vultr_instances": {},
    "port_forwards": {},
    "vpn": None,
    "vultr_api_key": None,
    **setup_config
}))

config = ConfigAccessor(config_manager, MullvadReloadHandler() )

# Write the config files
for service in SERVICES:
    service.write_config_file(config)

bash('systemctl daemon-reload')

# Enable and start the services
for service in SERVICES:
    service.enable()
    service.restart()