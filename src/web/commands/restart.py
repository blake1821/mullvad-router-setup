from time import sleep
from services.services import MULLVAD_SERVICE
from web.commands.program import Program
from web.context import RequestContext


class RestartProgram(Program):
    def execute(self, context: RequestContext) -> None:
        MULLVAD_SERVICE.restart()
        sleep(1)