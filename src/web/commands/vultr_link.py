from data.vultr import ApiKey
from web.commands.param_type import TextParamType
from web.commands.program import Program
from web.context import RequestContext


class LinkVultrProgram(Program):
    api_key = TextParamType(placeholder='9REHJR02JDJ2J2J3HJF02JDJFJ49FJN6NCUI').param('API Key')

    def execute(self, context: RequestContext) -> None:
        context.config.set_vultr_api_key(ApiKey(self.api_key))