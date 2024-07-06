from ipaddress import IPv4Address
from data.network import PortForward
from web.commands.program import Program
from web.commands.param_type import INT_PARAM_TYPE, TEXT_PARAM_TYPE
from web.context import RequestContext


class AddPortForwardProgram(Program):
    src_port = INT_PARAM_TYPE.param("Source Port")
    dst_addr = TEXT_PARAM_TYPE.param("Destination Address")
    dst_port = INT_PARAM_TYPE.param("Destination Port")

    def execute(self, context: RequestContext) -> None:
        config = context.config
        config.add_port_forward(PortForward(
            src_port=self.src_port,
            dst_addr=IPv4Address(self.dst_addr),
            dst_port=self.dst_port
        ), 'reload')