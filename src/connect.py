from dataclasses import dataclass
from ipaddress import IPv4Address, IPv4Interface, IPv6Interface
from typing import Optional
from config.config import ConfigManager
from data.mullvad import MullvadDevice
from data.network import PublicKey
from config.config_access import ConfigDAO
from network.firewall import forward_port, set_vpn_firewall
from network.ip import add_interface_ip, add_route_to_peer, bring_interface_up, reset_wg_interface, route_traffic_through_interface, set_forwarding_enabled
import random

from network.wg import set_wg
from services.services import NAME_SERVICE, PROXY_SERVICE, MullvadReloadHandler

@dataclass
class PeerConnection:
    my_ipv4_interface: IPv4Interface
    my_ipv6_interface: IPv6Interface
    ip: IPv4Address
    pubkey: PublicKey


config = ConfigDAO(ConfigManager(), MullvadReloadHandler())

reset_wg_interface(config.wg_ifname)

vpn = config.get_selected_vpn()
peer_ip: Optional[IPv4Address] = None
if vpn:
    peer: Optional[PeerConnection] = None
    if isinstance(vpn, MullvadDevice):
        relay = random.choice(vpn.relays)
        peer = PeerConnection(
            my_ipv4_interface=vpn.my_ipv4_interface,
            my_ipv6_interface=vpn.my_ipv6_interface,
            ip=relay.ip,
            pubkey=relay.pubkey
        )
    else:
        host = config.get_vultr_instance_host(vpn.id)
        if host:
            peer = PeerConnection(
                my_ipv4_interface=config.vultr_my_ipv4_interface,
                my_ipv6_interface=config.vultr_my_ipv6_interface,
                ip=host.ip,
                pubkey=vpn.public_key
            )
    
    if peer:
        add_interface_ip(
            ifname=config.wg_ifname,
            ipv4=peer.my_ipv4_interface,
            ipv6=peer.my_ipv6_interface
        )
        set_wg(
            wg_ifname=config.wg_ifname,
            private_key=config.private_key,
            peer_pubkey=peer.pubkey,
            peer_ip=peer.ip,
            wg_port=config.wg_port)
        peer_ip = peer.ip
        add_route_to_peer(peer_ip)

bring_interface_up(config.wg_ifname)
route_traffic_through_interface(config.wg_ifname)
set_forwarding_enabled(True)
set_vpn_firewall(
    wg=config.wg_ifname,
    lan=config.lan_ifname,
    man=config.man_ifname,
    wan=config.wan_ifname,
    peer_ip=peer_ip
)

for pf in config.get_port_forwards():
    forward_port(
        pf,
        incoming_interface=config.man_ifname
    )

NAME_SERVICE.write_config_file(config)
PROXY_SERVICE.restart()