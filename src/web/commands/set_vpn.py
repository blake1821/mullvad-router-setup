from web.commands.program import Program
from web.commands.param_type import VPN_PARAM_TYPE
from web.context import RequestContext


class SetVPNProgram(Program):
    vpn = VPN_PARAM_TYPE.param('VPN')

    def execute(self, context: RequestContext) -> None:
        context.config.set_selected_vpn(self.vpn, 'reload')
