from ipaddress import IPv4Address
from data.network import PortForward
from web.commands.program import Program
from web.commands.param_type import IntParamType, TextParamType
from web.context import RequestContext


class AddPortForwardProgram(Program):
    src_port = IntParamType(1022).param("Source Port")
    dst_addr = TextParamType(placeholder="192.168.0.15").param("Destination Address")
    dst_port = IntParamType(placeholder=22).param("Destination Port")

    def execute(self, context: RequestContext) -> None:
        config = context.config
        config.add_port_forward(PortForward(
            src_port=self.src_port,
            dst_addr=IPv4Address(self.dst_addr),
            dst_port=self.dst_port
        ), 'reload')