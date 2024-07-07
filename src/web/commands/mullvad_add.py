from ipaddress import IPv4Interface, IPv6Interface
from api.mullvad import get_mullvad_relays
from data.mullvad import MULLVAD_LOCATIONS, MullvadDevice
from network.firewall import set_basic_v4_firewall
from network.ip import remove_reroute, set_forwarding_enabled
from services.services import  MULLVAD_SERVICE, PROXY_SERVICE
from util import Id, get_random_string
from web.commands.param_type import SelectParamType, TextParamType
from web.commands.program import Program
from web.context import RequestContext


class AddMullvadDeviceProgram(Program):
    location = SelectParamType(
        id_map=MULLVAD_LOCATIONS
    ).param("Location")

    my_ipv4_interface = TextParamType(placeholder='10.42.192.33/32').param('IPv4')
    my_ipv6_interface = TextParamType(placeholder='fc00:bcbc::1/128').param('IPv6')
    
    def execute(self, context: RequestContext) -> None:
        config = context.config

        # Temporarily clear existing rules & routes,
        # so that we can connect to Mullvad's API
        PROXY_SERVICE.stop()
        set_forwarding_enabled(False)
        remove_reroute()
        set_basic_v4_firewall()

        device_id = get_random_string()
        config.add_mullvad_device(MullvadDevice(
            id=Id(device_id),
            location=self.location,
            my_ipv4_interface=IPv4Interface(self.my_ipv4_interface),
            my_ipv6_interface=IPv6Interface(self.my_ipv6_interface),
            relays=get_mullvad_relays(self.location)
        ))

        MULLVAD_SERVICE.restart()

