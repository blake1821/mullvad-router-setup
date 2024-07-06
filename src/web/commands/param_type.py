from abc import ABC, abstractmethod
from typing import Callable, Generic, Optional, TypeVar, cast

from data.mullvad import MullvadDevice
from config.config_access import ConfigAccessor
from util import Describable, Id, IdMap, Identifiable
from data.vpn import VPN

T = TypeVar('T')
class ParamType(Generic[T], ABC):
    @abstractmethod
    def build_html_input(self, *, name: str) -> str:
        raise NotImplementedError()

    @abstractmethod
    def parse_or_throw(self, value: str, config: ConfigAccessor) -> T:
        raise NotImplementedError()

    @abstractmethod
    def serialize(self, value: T) -> str:
        raise NotImplementedError()

    def param(self, display_name: str) -> T:
        """Do not override this!"""
        return cast(T, (self, display_name))


class TextParamType(ParamType[str]):
    def build_html_input(self, *, name: str) -> str:
        return f"""
            <input type="text" name="{name}" required>
        """

    def parse_or_throw(self, value: str, config: ConfigAccessor) -> str:
        return value

    def serialize(self, value: str) -> str:
        return value
TEXT_PARAM_TYPE = TextParamType()


class PasswordParamType(TextParamType):
    def build_html_input(self, *, name: str) -> str:
        return f"""
            <input type="password" name="{name}" required>
        """
PASSWORD_PARAM_TYPE = PasswordParamType()


class IntParamType(ParamType[int]):
    def build_html_input(self, *, name: str) -> str:
        return f"""
            <input type="number" step="1" name="{name}" required>
        """

    def parse_or_throw(self, value: str, config: ConfigAccessor) -> int:
        return int(value)

    def serialize(self, value: int) -> str:
        return str(value)
INT_PARAM_TYPE = IntParamType()

DI = TypeVar('DI', bound=Describable)
class SelectParamType(ParamType[DI]):
    def __init__(self, *,
                 id_map: IdMap[DI]):
        self.id_map = id_map

    def build_html_input(self, *, name: str) -> str:
        return f"""
            <select required name="{name}">{[
                f'<option value="{id}">{item.get_description()}</option>'
                for [id, item] in self.id_map.items()
            ]}</select>
        """
    
    def parse_or_throw(self, value: str, config: ConfigAccessor) -> DI:
        if value in self.id_map:
            return self.id_map[value]
        raise ValueError()

    def serialize(self, value: DI) -> str:
        return value.get_id()

I = TypeVar('I', bound=Identifiable)
class IdentifiableParamType(ParamType[I]):
    def build_html_input(self, *, name: str) -> str:
        raise RuntimeError("Cannot build HTML input for Identifiable Param")
    
    @abstractmethod
    def _find(self, id: Id[I], config: ConfigAccessor) -> Optional[I]:
        raise NotImplementedError()
    
    def parse_or_throw(self, value: str, config: ConfigAccessor) -> I:
        item = self._find(Id(value), config)
        if not item:
            raise ValueError(f'No item found: {value}')
        return item
    
    def serialize(self, value: I) -> str:
        return value.get_id()

class BasicIdentifiableParamType(IdentifiableParamType[I]):
    def __init__(self, lookup: Callable[[ConfigAccessor, Id[I]], Optional[I]]):
        self.lookup = lookup

    def _find(self, id: Id[I], config: ConfigAccessor) -> I | None:
        return self.lookup(config, id)

MULLVAD_DEVICE_PARAM_TYPE = BasicIdentifiableParamType(lambda config, id: config.get_mullvad_device(id))
VULTR_INSTANCE_PARAM_TYPE = BasicIdentifiableParamType(lambda config, id: config.get_vultr_instance(id))

class VPNParamType(ParamType[Optional[VPN]]):
    def build_html_input(self, *, name: str) -> str:
        raise RuntimeError("Cannot build HTML input for VPN Param")
    
    def parse_or_throw(self, value: str, config: ConfigAccessor) -> Optional[VPN]:
        if value == 'none':
            return None

        [vpn_type, vpn_id] = value.split(':')
        vpn: Optional[VPN] = None
        if vpn_type == 'mullvad':
            vpn = config.get_mullvad_device(Id(vpn_id))
        elif vpn_type == 'vultr':
            vpn = config.get_vultr_instance(Id(vpn_id))
        
        if not vpn:
            raise ValueError(f'No VPN found in config: {value}')

        return vpn
    
    def serialize(self, value: Optional[VPN]) -> str:
        if value is None:
            return 'none'

        if isinstance(value, MullvadDevice):
            return f'mullvad:{value.id}'
        else:
            return f'vultr:{value.id}'

VPN_PARAM_TYPE = VPNParamType()