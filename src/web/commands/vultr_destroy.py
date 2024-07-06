from api.vultr import VultrAPI
from web.commands.param_type import VULTR_INSTANCE_PARAM_TYPE
from web.commands.program import Program
from web.context import RequestContext


class DestroyVultrInstanceProgram(Program):
    instance = VULTR_INSTANCE_PARAM_TYPE.param('Vultr Instance')

    def execute(self, context: RequestContext) -> None:
        config = context.config
        api_key = config.get_vultr_api_key()
        assert api_key
        api = VultrAPI(api_key)
        api.destroy_instance(self.instance.id)
        config.delete_vultr_instance(self.instance.id, 'reload')

