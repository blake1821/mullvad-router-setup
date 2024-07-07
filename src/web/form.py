from web.commands.program import Program, CommandParam, Command
from web.pages.html_util import render_html_table
from web.route import CommandRoute

from abc import ABC, abstractmethod
from typing import Any, Optional, TypeVar


P = TypeVar('P', bound=Program)
class HTMLFormBuilder(ABC):
    def build(self,
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

        return f'''
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


class HTMLTableFormBuilder(HTMLFormBuilder):
    def __init__(self, submit_text: str):
        self.submit_text = submit_text

    def _build_inside(self, params: list[CommandParam[Any]]) -> str:
        return f'''
            {render_html_table([
                [param.display_name+": ", param.build_html_input()]
                for param in params
            ])}
            <input class="margin-top" type="submit" value="{self.submit_text}">
        '''


class ButtonFormBuilder(HTMLFormBuilder):
    def __init__(self, submit_text: str, margin_top: bool = False):
        self.submit_text = submit_text
        self.margin_top = margin_top

    def _build_inside(self, params: list[CommandParam[Any]]) -> str:
        assert len(params) == 0
        return f'''
            <input class="{"margin-top" if self.margin_top else "" }" type="submit" value="{self.submit_text}">
        '''