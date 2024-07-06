from web.commands.program import Program
from web.commands.param_type import INT_PARAM_TYPE
from web.context import RequestContext


class DeletePortForwardProgram(Program):
    src_port = INT_PARAM_TYPE.param("Source Port")

    def execute(self, context: RequestContext) -> None:
        config = context.config
        config.delete_port_forward(self.src_port, 'reload')