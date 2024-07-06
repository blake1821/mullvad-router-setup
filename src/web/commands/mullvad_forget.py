from web.commands.param_type import MULLVAD_DEVICE_PARAM_TYPE
from web.commands.program import Program
from web.context import RequestContext



class ForgetMullvadDeviceProgram(Program):
    device = MULLVAD_DEVICE_PARAM_TYPE.param('Mullvad Device')

    def execute(self, context: RequestContext) -> None:
        config = context.config
        config.delete_mullvad_device(self.device.id, 'reload')
