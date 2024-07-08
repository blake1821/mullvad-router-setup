from dataclasses import dataclass
from ipaddress import IPv4Address
from typing import NewType, Self

from data.network import PublicKey
from util.util import Describable, Id, Identifiable, create_id_map

@dataclass
class VultrInstanceHost:
    ip: IPv4Address
    password: str

@dataclass
class VultrInstance(Identifiable):
    id: Id[Self]
    public_key: PublicKey

    def get_id(self) -> Id[Self]:
        return self.id

@dataclass
class VultrPlan(Describable):
    id: Id[Self]
    description: str

    def get_id(self) -> Id[Self]:
        return self.id

    def get_description(self) -> str:
        return self.description


@dataclass
class VultrLocation(Describable):
    id: Id[Self]
    name: str

    def get_id(self) -> Id[Self]:
        return self.id

    def get_description(self) -> str:
        return self.name


class VultrOS():
    def __init__(self, id: int, name: str):
        self.id = id
        self.name = name


VULTR_PLANS = create_id_map([
   VultrPlan(Id("vc2-1c-1gb"), description="VC2, 1c, 1GB, 1TB Bandwidth, $5/month"),
   VultrPlan(Id("vhp-1c-1gb-amd"), description="VHP-AMD, 1c, 1GB, 2TB Bandwidth, $6/month")
])
VULTR_LOCATIONS = create_id_map([
    VultrLocation(Id("ewr"), "New Jersey"),
    VultrLocation(Id("ord"), "Chicago"),
    VultrLocation(Id("dfw"), "Dallas"),
    VultrLocation(Id("sea"), "Seattle"),
    VultrLocation(Id("lax"), "Los Angeles"),
    VultrLocation(Id("atl"), "Atlanta"),
    VultrLocation(Id("sjc"), "Silicon Valley"),
    VultrLocation(Id("yto"), "Toronto"),
    VultrLocation(Id("mia"), "Miami")
])
VULTR_DEBIAN_12 = VultrOS(2136, 'Debian 12')
ApiKey = NewType('ApiKey', str)