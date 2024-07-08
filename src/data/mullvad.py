
from dataclasses import dataclass
from ipaddress import IPv4Address, IPv4Interface, IPv6Interface
from typing import Self

from data.network import PublicKey
from util.util import Describable, Id, Identifiable, create_id_map


@dataclass
class MullvadRelay:
    """Mullvad wireguard peer"""
    ip: IPv4Address
    pubkey: PublicKey


@dataclass
class MullvadLocation(Describable):
    """Mullvad server location"""
    id: Id[Self]
    name: str
    prefix: str
    
    def get_id(self) -> Id[Self]:
        return self.id
    
    def get_prefix(self) -> str:
        return self.prefix

    def get_description(self) -> str:
        return self.name


@dataclass
class MullvadDevice(Identifiable):
    id: Id[Self]
    location: MullvadLocation
    my_ipv4_interface: IPv4Interface
    my_ipv6_interface: IPv6Interface
    relays: list[MullvadRelay]

    def get_id(self) -> Id[Self]:
        return self.id


MULLVAD_LOCATIONS = create_id_map([
    MullvadLocation(Id("anywhere"), "Anywhere", ''),
    MullvadLocation(Id("us"), "US", 'us')
])
        
    
                
