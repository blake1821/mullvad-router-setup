from web.commands.program import Program, CommandParam, Command
from web.pages.html_util import get_html_table
from web.route import CommandRoute

from abc import ABC, abstractmethod
from typing import Any, Generic, Optional, TypeVar


P = TypeVar('P', bound=Program)
class HTMLForm(ABC, Generic[P]):
    def __init__(self,
                 command_route: CommandRoute[P],
                 *,
                 command: Optional[Command[P]] = None):
        assert command_route.method == 'POST'
        program = command_route.handler.program
        path = command_route.path

        if not command:
            hidden_args = {}
            unassigned_params = program.get_params()
        else:
            hidden_args = command.serialize_args()
            unassigned_params = command.get_unassigned_params()

        self.html = f'''
        <form method="POST" action="{path}">
            {"".join(
                f'<input type="hidden" name="{name}" value={value}>'
                for name, value in hidden_args.items()
            )}
            {
                self._build_inside([
                    param
                    for param in unassigned_params
                ])
            }
        </form>'''


    @abstractmethod
    def _build_inside(self, params: list[CommandParam[Any]]) -> str:
        pass


class HTMLTableForm(HTMLForm[P]):
    def __init__(self,
                 command_route: CommandRoute[P],
                 submit_text: str,
                 *,
                 command: Optional[Command[P]] = None):
        self.submit_text = submit_text
        super().__init__(command_route, command=command)

    def _build_inside(self, params: list[CommandParam[Any]]) -> str:
        return f'''
            {get_html_table([
                [param.display_name, param.build_html_input()]
                for param in params
            ])}
            <input class="margin-top" type="submit" value="{self.submit_text}">
        '''


class SubmitButtonForm(HTMLForm[P]):
    def __init__(self,
                 command_route: CommandRoute[P],
                 submit_text: str,
                 *,
                 command: Optional[Command[P]] = None,
                 margin_top: bool = False):
        self.submit_text = submit_text
        self.margin_top = margin_top
        super().__init__(command_route, command=command)

    def _build_inside(self, params: list[CommandParam[Any]]) -> str:
        assert len(params) == 0
        return f'''
            <input class="{"margin-top" if self.margin_top else "" }" type="submit" value="{self.submit_text}">
        '''