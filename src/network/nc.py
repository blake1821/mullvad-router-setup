from ipaddress import IPv4Address
from util import bash


def is_tcp_port_open(address: IPv4Address, port: int):
    return bash(f'nc -vz {address} -w 5 {port}')