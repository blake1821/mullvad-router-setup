

from types import GenericAlias, NoneType
from typing import Any, Generic, Literal, TypeVar, TypedDict, Union, cast


# Get metaclasses of some common types
T = TypeVar('T')
class ExampleTypedDict(TypedDict):
    pass
class ExampleGeneric(Generic[T]):
    pass
TYPED_DICT_META = type(ExampleTypedDict)
UNION_META = type(Union[int, str])
LITERAL_META = type(Literal['3'])
GENERIC_META = type(ExampleGeneric[str])

BaseType = type[Any] | Literal["TypedDict"] | Literal['Optional'] | Literal['Literal']

class BaseTypeDescription(Generic[T]):
    base_type: BaseType

class ClassTypeDescription(BaseTypeDescription[T]):
    """
    This class represents a class together with its type arguments.
    For example, the type `list[int]` would be represented as:
    - base_type = list
    - type_args = [int]

    Unfortunately, Python's typing module does not provide a uniform way to decompose types like this.
    """

    def __init__(self, base_type: type | Literal['Optional'], type_args: list[type]):
        self.base_type = base_type
        self.type_args = type_args
    
    def __eq__(self, other: object) -> bool:
        return isinstance(other, ClassTypeDescription) and self.base_type == other.base_type and self.type_args == other.type_args
    
    def __hash__(self) -> int:
        return hash((self.base_type, tuple(self.type_args)))

class TypedDictDescription(BaseTypeDescription[T]):
    """TypedDict analog of FullySpecifiedClass"""
    base_type = 'TypedDict'

    def __init__(self, typed_dict_type: type) -> None:
        self.typed_dict_type = cast(type[T], typed_dict_type)

class LiteralTypeDescription(BaseTypeDescription[T]):
    """Literal analog of FullySpecifiedClass"""
    base_type = 'Literal'

    def __init__(self, literal_options: list[object]) -> None:
        self.literal_options = literal_options

TypeDescription = ClassTypeDescription[T] | TypedDictDescription[T] | LiteralTypeDescription[T]

def get_type_description(t: type[T]) -> TypeDescription[T]:
    if type(t) == TYPED_DICT_META:
        return TypedDictDescription(t)
    elif isinstance(t, GenericAlias):
        if t.__origin__ == list:
            return ClassTypeDescription(list, list(t.__args__))
        elif t.__origin__ == dict:
            return ClassTypeDescription(dict, list(t.__args__))
        else:
            raise ValueError(f'Unknown Generic Alias {t}')
    elif type(t) == GENERIC_META: 
        origin_type: type = vars(t)['__origin__']
        return ClassTypeDescription(origin_type, list(t.__args__)) # type: ignore
    elif type(t) == UNION_META:
        args = cast(tuple[type, ...], t.__args__) # type: ignore
        # Test for optional
        if len(args) == 2 and args[1] == NoneType:
            return ClassTypeDescription('Optional', list(args))
        else:
            raise ValueError(f'Unions like {t} do not have a fully specified type yet.')
    elif type(t) == LITERAL_META:
        args = cast(tuple[object, ...], t.__args__)  # type: ignore
        return LiteralTypeDescription(list(args))
    else:
        return ClassTypeDescription[T](t, [])


# JSON Object
JsonAtom = int | str | bool | None.__class__
JsonObj = JsonAtom | dict[str, "JsonObj"] | list["JsonObj"]