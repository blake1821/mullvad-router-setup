from ipaddress import IPv4Address
from data.network import PrivateKey
from data.network import PublicKey
from util.bash import bash


def set_wg(*,
           wg_ifname: str,
           private_key: PrivateKey,
           peer_pubkey: PublicKey,
           peer_ip: IPv4Address,
           wg_port: int):
    bash(f'wg set {wg_ifname} \
        listen-port {wg_port} \
        private-key <(echo {private_key}) \
        peer {peer_pubkey} \
            endpoint {peer_ip}:{wg_port} \
            allowed-ips 0.0.0.0/0,::/0')