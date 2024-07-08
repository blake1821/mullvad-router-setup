from types import SimpleNamespace
from typing import Any, Generic, Literal, Optional, TypeVar, cast

from config.config_access import ConfigDAO
from web.commands.param_type import ParamType
from web.context import RequestContext

T = TypeVar('T')
class CommandParam(Generic[T]):
    def __init__(self, *,
                 name: str,
                 display_name: str,
                 type: ParamType[T],
                 placeholder: Optional[str] = None):
        self.name = name
        self.display_name = display_name
        self.type = type
        self.placeholder = placeholder
    
    def build_html_input(self):
        return self.type.build_html_input(name=self.name)
    
class Program:
    """Extend this class and add params like this:
        ```
        class_variable = INT_PARAM_TYPE.param()
        ...
        ```

        Do not override the constructor.

        Then implement execute(self). self will have all params
        instantiated.
    """

    def __init__(self,
                 s: Literal['Do not call this constructor']):
        pass

    @classmethod
    def get_params(cls) -> list[CommandParam[Any]]:
        return [
            CommandParam[Any](
                name=name,
                display_name=display_name,
                type=cast(ParamType[Any], param)
            )
            for [name, v] in cls.__dict__.items()
            if not name.startswith('__') and not callable(v)
            for [param, display_name] in [v]
        ]
    
    
    def execute(self, context: RequestContext) -> None:
        raise NotImplementedError()


        
P = TypeVar('P', bound=Program) 
class Command(Generic[P]):
    def __init__(self, program: type[P]):
        self.args = cast(P, SimpleNamespace())
        self.program = program
    
    def serialize_args(self) -> dict[str, str]:
        params = self.program.get_params()
        return {
            param.name: param.type.serialize(self.args.__dict__[param.name])
            for param in params
            if param.name in self.args.__dict__
        }
    
    def load_args(self, serialized: dict[str, str], config: ConfigDAO):
        for param in self.program.get_params():
            self.args.__dict__[param.name] = param.type.parse_or_throw(serialized[param.name], config)
    
    def get_unassigned_params(self) -> list[CommandParam[Any]]:
        return [
            param
            for param in self.program.get_params()
            if param.name not in self.args.__dict__
        ]
    
    def execute(self, context: RequestContext):
        process = object.__new__(self.program)
        process.__dict__.update(self.args.__dict__)
        process.execute(context)

