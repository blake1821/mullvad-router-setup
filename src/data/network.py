from dataclasses import dataclass
from ipaddress import IPv4Address
from typing import NewType

from util.bash import bash_get

@dataclass
class PortForward:
    src_port: int
    dst_addr: IPv4Address
    dst_port: int

IfName = NewType('IfName', str)

PublicKey = NewType('PublicKey', str)

class PrivateKey(str):
    @classmethod
    def generate(cls):
        return PrivateKey(bash_get('wg genkey'))

    def get_public_key(self):
        return PublicKey(bash_get(f'echo {self} |  wg pubkey'))