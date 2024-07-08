from abc import ABC, abstractmethod
from ipaddress import IPv4Address, IPv4Interface, IPv6Address, IPv6Interface
from typing import Iterable, Literal, Optional
from config.config import Config, ConfigManager, PortForwardCfg, VPNSelectionCfg, VultrInstanceCfg
from data.mullvad import MullvadDevice
from data.network import IfName, PortForward, PrivateKey
from util.util import Id
from data.vultr import ApiKey, VultrInstance, VultrInstanceHost
from data.vpn import VPN

class ReloadHandler(ABC):
    @abstractmethod
    def reload(self, reload: Literal['reload']) -> None:
        """Called whenever the config file is updated in a way that requires a service restart"""

        raise NotImplementedError()
    

class ConfigDAO:
    """Data Access Object for the config file"""

    def __init__(self, config_manager: ConfigManager, reload_handler: ReloadHandler):
        self._config_manager = config_manager
        self.reload_handler = reload_handler
    
    def _load(self):
        return self._config_manager.load()
    
    def _save(self, config: Config):
        self._config_manager.save(config)
    
    def _reload(self, reload: Literal['reload']):
        self.reload_handler.reload(reload)

    def _save_and_reload(self, config: Config, reload: Literal['reload']):
        self._save(config)
        self._reload(reload)
    
    def _enforce_referential_integrity(self, reload: Literal['reload']):
        config = self._load()
        vpn = config['vpn']
        if vpn:
            if self._get_vpn(vpn) == None:
                self.set_selected_vpn(None, reload)

    @property
    def dns(self) -> IPv4Address:
        return self._load()['dns']

    @property
    def dns6(self) -> IPv6Address:
        return self._load()['dns6']

    @property
    def lan_v4_interface(self) -> IPv4Interface:
        return self._load()['lan_v4_interface']

    @property
    def lan_v6_interface(self) -> IPv6Interface:
        return self._load()['lan_v6_interface']

    @property
    def wg_ifname(self) -> IfName:
        return self._load()['wg_ifname']

    @property
    def proxy_port(self) -> int:
        return self._load()['proxy_port']

    @property
    def wg_port(self) -> int:
        return self._load()['wg_port']

    @property
    def admin_port(self) -> int:
        return self._load()['admin_port']

    @property
    def vultr_my_ipv4_interface(self) -> IPv4Interface:
        return self._load()['vultr_my_ipv4_interface']

    @property
    def vultr_my_ipv6_interface(self) -> IPv6Interface:
        return self._load()['vultr_my_ipv6_interface']

    
    def get_mullvad_devices(self) -> Iterable[MullvadDevice]:
        return self._load()['mullvad_devices'].values()
    
    def get_mullvad_device(self, id: Id[MullvadDevice]) -> MullvadDevice | None:
        d = self._load()['mullvad_devices']
        return d[id] if id in d else None
    
    def add_mullvad_device(self, mullvad_device: MullvadDevice):
        config = self._load()
        config['mullvad_devices'][mullvad_device.id] = mullvad_device
        self._save(config)
    
    def delete_mullvad_device(self, id: Id[MullvadDevice], reload: Literal['reload']):
        config = self._load()
        del config['mullvad_devices'][id] 
        self._save(config)
        self._enforce_referential_integrity(reload)

    def get_vultr_instances(self) -> Iterable[VultrInstance]:
        return (
            VultrInstance(
                instance['id'],
                instance['pubkey']
            )
            for instance in self._load()['vultr_instances'].values()
        )
    
    def get_vultr_instance(self, id: Id[VultrInstance]) -> VultrInstance | None:
        d = self._load()['vultr_instances']
        if id not in d:
            return None
        instance = d[id]
        return VultrInstance(
            id=instance['id'],
            public_key=instance['pubkey']
        )
    
    def get_vultr_instance_host(self, id: Id[VultrInstance]) -> VultrInstanceHost | None:
        instance = self._load()['vultr_instances'][id]
        return instance['host'] if instance else None

    def add_vultr_instance(self, vultr_instance: VultrInstance):
        config = self._load()
        config['vultr_instances'][vultr_instance.id] = VultrInstanceCfg({
            'id': vultr_instance.get_id(),
            'pubkey': vultr_instance.public_key,
            'host': None
        })
        self._save(config)

    def set_vultr_instance_host(self, id: Id[VultrInstance], host: VultrInstanceHost):
        config = self._load()
        assert id in config['vultr_instances']
        config['vultr_instances'][id]['host'] = host
        self._save(config)
    
    def delete_vultr_instance(self, id: Id[VultrInstance], reload: Literal['reload']):
        config = self._load()
        del config['vultr_instances'][id] 
        self._save(config)
        self._enforce_referential_integrity(reload)
    
    def get_port_forwards(self) -> list[PortForward]:
        return [
            PortForward(
                int(src_port),
                dst_addr=pf['dst_addr'],
                dst_port=pf['dst_port']
            )
            for src_port, pf in self._load()['port_forwards'].items()
        ]
    
    def add_port_forward(self, pf: PortForward, reload: Literal['reload']):
        config = self._load()
        config['port_forwards'][str(pf.src_port)] = PortForwardCfg(
            dst_addr=pf.dst_addr,
            dst_port=pf.dst_port
        )
        self._save_and_reload(config, reload)

    def delete_port_forward(self, src_port: int, reload: Literal['reload']):
        config = self._load()
        del config['port_forwards'][str(src_port)]
        self._save_and_reload(config, reload)
    
    def _get_vpn(self, vpn: VPNSelectionCfg) -> VPN | None:
        if vpn['type'] == 'mullvad':
            return self.get_mullvad_device(vpn['id'])
        else:
            return self.get_vultr_instance(vpn['id'])

    def get_selected_vpn(self) -> VPN | None:
        config = self._load()
        vpn = config['vpn']
        if vpn:
            return self._get_vpn(vpn)
        return None
    
    def set_selected_vpn(self, vpn: VPN | None, reload: Literal['reload']):
        config = self._load()
        if vpn == None: 
            config['vpn'] = None
        elif isinstance(vpn, VultrInstance):
            config['vpn'] = VPNSelectionCfg(type='vultr', id=vpn.id)
        else:
            config['vpn'] = VPNSelectionCfg(type='mullvad', id=vpn.id)
        self._save_and_reload(config, reload)
    

    @property
    def wan_ifname(self) -> IfName:
        return self._load()['wan_ifname']

    @property
    def man_ifname(self) -> IfName:
        return self._load()['man_ifname']

    @property
    def lan_ifname(self) -> IfName:
        return self._load()['lan_ifname']

    @property
    def router_password(self) -> str:
        return self._load()['router_password']

    @property
    def private_key(self) -> PrivateKey:
        return self._load()['private_key']

    @property
    def login_cookie(self) -> str:
        return self._load()['login_cookie']

    def get_vultr_api_key(self) -> Optional[ApiKey]:
        return self._load()['vultr_api_key']

    def set_vultr_api_key(self, api_key: ApiKey):
        config = self._load()
        config['vultr_api_key'] = api_key
        self._save(config)



    