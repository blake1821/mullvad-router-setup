from ipaddress import IPv4Address, IPv4Interface, IPv6Address, IPv6Interface
import os
import json
from typing import Any, Literal, Optional, TypedDict

from config.transformer import Transformer, BaseTransformerRegistry, TransformerRegistry
from data.mullvad import MULLVAD_LOCATIONS, MullvadDevice, MullvadLocation, MullvadRelay
from data.network import IfName, PrivateKey, PublicKey
from data.vultr import ApiKey, VultrInstance, VultrInstanceHost
from util.typing import JsonObj, TypeDescription
from util.util import Id

CONFIG_FILENAME = 'config.json'

# Config-file-only objects

class VultrInstanceCfg(TypedDict):
    host: Optional[VultrInstanceHost]
    id: Id[VultrInstance]
    pubkey: PublicKey

class VPNSelectionCfg(TypedDict):
    type: Literal['mullvad', 'vultr']
    id: Id[Any]

class PortForwardCfg(TypedDict):
    dst_addr: IPv4Address
    dst_port: int

# A setup config is the subset of a config that is used for the setup script
# It is not sufficient to run the other scripts

class SetupConfig(TypedDict):
    dns: IPv4Address
    dns6: IPv6Address
    lan_v4_interface: IPv4Interface
    lan_v6_interface: IPv6Interface
    wg_ifname: IfName
    proxy_port: int
    wg_port: int
    admin_port: int
    vultr_my_ipv4_interface: IPv4Interface
    vultr_my_ipv6_interface: IPv6Interface

def get_default_setup_config() -> SetupConfig:
    return {
        'dns': IPv4Address('1.1.1.1'),
        'dns6': IPv6Address('2606:4700:4700::1111'),
        'lan_v4_interface': IPv4Interface('192.168.0.1/24'),
        'lan_v6_interface': IPv6Interface('2001:cccc::1/64'),
        'wg_ifname': IfName('wg0'),
        'proxy_port': 1080,
        'wg_port': 51820,
        'admin_port': 80,
        'vultr_my_ipv4_interface': IPv4Interface('10.0.33.2/24'),
        'vultr_my_ipv6_interface': IPv6Interface('fc00:3333::2/120')
    }

class Config(SetupConfig):
    mullvad_devices: dict[Id[MullvadDevice], MullvadDevice]
    vultr_instances: dict[Id[VultrInstance], VultrInstanceCfg]
    port_forwards: dict[str, PortForwardCfg]
    vpn: Optional[VPNSelectionCfg]
    wan_ifname: IfName
    man_ifname: IfName
    lan_ifname: IfName
    router_password: str
    private_key: PrivateKey
    login_cookie: str
    vultr_api_key: Optional[ApiKey]

# Create the data transformers for the config files
# (Recall that these are used to convert between python classes and the JSON representation)

class MullvadLocationTransformer(Transformer[MullvadLocation]):
    """Transforms a MullvadLocation to and from a string id"""

    def parse(self, json: JsonObj) -> MullvadLocation:
        assert isinstance(json, str)
        return MULLVAD_LOCATIONS[Id(json)]
    
    def serialize(self, value: MullvadLocation) -> JsonObj:
        return value.get_id()

    @classmethod
    def create(cls, type_description: TypeDescription[MullvadLocation], transformer_registry: BaseTransformerRegistry) -> Transformer[MullvadLocation]:
        return MullvadLocationTransformer()
    
    

config_transformer_registry = TransformerRegistry()\
    .register_stringlike(IPv4Address)\
    .register_stringlike(IPv6Address)\
    .register_stringlike(IPv4Interface)\
    .register_stringlike(IPv6Interface)\
    .register_stringlike(PublicKey)\
    .register_stringlike(PrivateKey)\
    .register_stringlike(Id)\
    .register_stringlike(IfName)\
    .register_stringlike(ApiKey)\
    .register_dataclass(MullvadRelay)\
    .register_dataclass(MullvadDevice)\
    .register_dataclass(VultrInstanceHost)\
    .register_transformer(MullvadLocation, MullvadLocationTransformer.create)

CONFIG_TRANSFORMER = config_transformer_registry.get_transformer(Config)
SETUP_CONFIG_TRANSFORMER = config_transformer_registry.get_transformer(SetupConfig)

class ConfigManager:
    """Singleton class that saves and loads the config file"""

    def __init__(self):
        self._cache : Optional[Config] = None
    
    def load(self):
        if not self._cache:
            if not os.path.exists(CONFIG_FILENAME):
                raise FileNotFoundError('Cannot find config file')
            
            with open(CONFIG_FILENAME, 'r') as config_file:
                config_data = json.load(config_file)
            
            self._cache = CONFIG_TRANSFORMER.parse(config_data)
        return self._cache

    def save(self, config: Config):
        serialized_config = json.dumps(CONFIG_TRANSFORMER.serialize(config), indent=4)
        with open(CONFIG_FILENAME, 'w') as config_file:
            config_file.write(serialized_config)
        self._cache = config
        

def init_config():
    """Create a setup config file with default values"""
    with open(CONFIG_FILENAME, 'w') as setup_config_file:
        json.dump(
            SETUP_CONFIG_TRANSFORMER.serialize(get_default_setup_config()),
            setup_config_file,
            indent=4
        )

def get_setup_config() -> SetupConfig:
    """Get the setup config from the config file, or create it if it doesn't exist"""
    if not os.path.exists(CONFIG_FILENAME):
        init_config()
    
    with open(CONFIG_FILENAME, 'r') as config_file:
        config_data = json.load(config_file)
    
    return SETUP_CONFIG_TRANSFORMER.parse(config_data)

