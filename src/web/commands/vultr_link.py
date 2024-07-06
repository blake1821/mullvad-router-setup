from data.vultr import ApiKey
from web.commands.param_type import TEXT_PARAM_TYPE
from web.commands.program import Program
from web.context import RequestContext


class LinkVultrProgram(Program):
    api_key = TEXT_PARAM_TYPE.param('API Key')

    def execute(self, context: RequestContext) -> None:
        context.config.set_vultr_api_key(ApiKey(self.api_key))