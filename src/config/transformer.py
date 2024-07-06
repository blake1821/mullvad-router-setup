from abc import ABC, abstractmethod
from dataclasses import fields, is_dataclass
from types import GenericAlias, NoneType
from typing import Any, Generic, Literal, Optional, Protocol, TypeVar, TypedDict, Union, cast, get_type_hints



JsonObj = int | str | bool | None.__class__ | dict[str, "JsonObj"] | list["JsonObj"]

T = TypeVar('T')
SerT = TypeVar('SerT')
JT = TypeVar('JT', bound=int | str | bool | None.__class__)


class IntermediateTransformer(ABC, Generic[SerT, T]):
    def __init__(self, ser_t: type[SerT], t: type[T]):
        self.ser_t = ser_t
        self.t = t

    @abstractmethod
    def parse(self, ser: SerT) -> T:
        raise NotImplementedError()

    @abstractmethod
    def serialize(self, value: T) -> SerT:
        raise NotImplementedError()


class LeafTransformer(ABC, Generic[T]):
    def __init__(self, t: type[T]):
        self.t = t

    @abstractmethod
    def parse(self, json: JsonObj) -> T:
        raise NotImplementedError()

    @abstractmethod
    def serialize(self, value: T) -> JsonObj:
        raise NotImplementedError()

Transformer = IntermediateTransformer[Any, T] | LeafTransformer[T]

class JsonAtomTransformer(LeafTransformer[JT]):
    def parse(self, json: JsonObj) -> JT:
        assert isinstance(json, self.t)
        return json

    def serialize(self, value: JT) -> JsonObj:
        return value

class StringInit(Protocol):
    def __init__(self, _x: str) -> None:
        ...

SI = TypeVar('SI', bound=StringInit)
class StringInitTransformer(LeafTransformer[SI]):
    def parse(self, json: JsonObj) -> SI:
        assert isinstance(json, str)
        return self.t(json)
    
    def serialize(self, value: SI) -> JsonObj:
        return str(value)

ATOM_TRANSFORMERS : list[JsonAtomTransformer[Any]] = [
    JsonAtomTransformer(int),
    JsonAtomTransformer(str),
    JsonAtomTransformer(bool),
    JsonAtomTransformer(None.__class__)
]

class ExampleTypedDict(TypedDict):
    pass
class ExampleGeneric(Generic[T]):
    pass
TYPED_DICT_META = type(ExampleTypedDict)
UNION_META = type(Union[int, str])
LITERAL_META = type(Literal['3'])
GENERIC_META = type(ExampleGeneric[str])

class TypedDictTransformer(LeafTransformer[dict[str, Any]]):
    def __init__(self,
                 attribute_transormers: dict[str, LeafTransformer[Any]]):
        self.attribute_transformers = attribute_transormers

    def parse(self, json: JsonObj) -> dict[str, Any]:
        assert isinstance(json, dict)
        return {
            k: at.parse(json[k])
            for k, at in self.attribute_transformers.items()
        }
    
    def serialize(self, value: dict[str, Any]) -> JsonObj:
        return {
            k: at.serialize(value[k])
            for k, at in self.attribute_transformers.items()
        }

class ListTransformer(LeafTransformer[list[T]]):
    def __init__(self,
                 attribute_transormer: LeafTransformer[T]):
        self.attribute_transformer = attribute_transormer

    def parse(self, json: JsonObj) -> list[T]:
        assert isinstance(json, list)
        return [
            self.attribute_transformer.parse(item)
            for item in json
        ]
    
    def serialize(self, value: list[T]) -> JsonObj:
        return [
            self.attribute_transformer.serialize(item)
            for item in value
        ]

class DictTransformer(LeafTransformer[dict[str, T]]):
    def __init__(self,
                 attribute_transormer: LeafTransformer[T]):
        self.attribute_transformer = attribute_transormer

    def parse(self, json: JsonObj) -> dict[str, T]:
        assert isinstance(json, dict)
        return {
            k: self.attribute_transformer.parse(v)
            for k, v in json.items()
        }
    
    def serialize(self, value: dict[str, T]) -> JsonObj:
        return {
            k: self.attribute_transformer.serialize(v)
            for k, v in value.items()
        }

class OptionalTransformer(LeafTransformer[Optional[T]]):
    def __init__(self,
                 attribute_transormer: LeafTransformer[T]):
        self.attribute_transformer = attribute_transormer

    def parse(self, json: JsonObj) -> Optional[T]:
        if json is None:
            return None
        return self.attribute_transformer.parse(json)
    
    def serialize(self, value: Optional[T]) -> JsonObj:
        if value is None:
            return None
        return self.attribute_transformer.serialize(value)

class CompositionalTransformer(Generic[SerT, T], LeafTransformer[T]):
    def __init__(self,
                 outer: IntermediateTransformer[SerT, T],
                 inner: LeafTransformer[SerT]):
        super().__init__(outer.t)
        self.outer = outer
        self.inner = inner
    
    def parse(self, json: JsonObj) -> T:
        return self.outer.parse(self.inner.parse(json))

    def serialize(self, value: T) -> JsonObj:
        return self.inner.serialize(self.outer.serialize(value))

class DataclassTransformer(IntermediateTransformer[Any, T]):
    def __init__(self, t: type[T]):
        assert is_dataclass(t)

        self.datadict = TypedDict('datadict', {
            field.name: field.type
            for field in fields(t) # type: ignore
        })

        super().__init__(self.datadict, t)
    
    def parse(self, ser: Any) -> T:
        return self.t(**ser)
    
    def serialize(self, value: T) -> Any:
        return self.datadict({
            field.name: vars(value)[field.name]
            for field in fields(self.t) #type: ignore
        }) # type: ignore



# def CustomConfigObject(transformer: LeafTransformer[T]) -> type[T]:
#     return cast(type[T], transformer)

# Might be useful later
# class Typed(TypedDict):
#     type: str

# U = TypeVar('U', bound=Typed)
# def ConfigUnionType(*x: tuple[str, type[U]]) -> type[U]:
#     pass

# class MullvadVPN(TypedDict):
#     type: str
#     id: Id[MullvadDevice]

# class VultrVPN(TypedDict):
#     type: str
#     id: Id[VultrInstance]

# x = ConfigUnionType(('s', MullvadVPN), ('d', VultrVPN))

class NestedTransfomer(LeafTransformer[T]):

    def _get_transformer(self, t: type) -> LeafTransformer[Any]:
        if t in self.type_transformer:
            t_transformer = self.type_transformer[t]
            if isinstance(t_transformer, LeafTransformer):
                return t_transformer
            else:
                ser_t_transformer = self._get_transformer(t_transformer.ser_t)
                return CompositionalTransformer(
                    t_transformer,
                    ser_t_transformer
                )
        elif type(t) == TYPED_DICT_META:
            attribute_transformers = {
                k: self._get_transformer(at)
                for k, at in get_type_hints(t).items()
            }
            return TypedDictTransformer(attribute_transformers) # type: ignore
        elif isinstance(t, GenericAlias):
            if t.__origin__ == list:
                return ListTransformer(self._get_transformer(t.__args__[0]))
            elif t.__origin__ == dict:
                assert isinstance(t.__args__[0](''), str)
                return DictTransformer(self._get_transformer(t.__args__[1]))
            else:
                raise ValueError(f'Unknown Generic Alias {t}')
        elif type(t) == GENERIC_META: 
            origin_type: type = vars(t)['__origin__']
            return self._get_transformer(origin_type)
        elif type(t) == UNION_META:
            args = cast(tuple[type, ...], t.__args__)  # type: ignore
            # Test for optional
            if len(args) == 2 and args[1] == NoneType:
                return OptionalTransformer(self._get_transformer(args[0]))
            else:
                raise ValueError(f'Unions like {t} are not transformable. Try using a CustomConfigObject')
        elif type(t) == LITERAL_META:
            args = cast(tuple[type, ...], t.__args__)  # type: ignore
            if all(isinstance(arg, str | int | None | bool) for arg in args):
                return JsonAtomTransformer(str | int | None | bool)
            raise ValueError(f'Literals like {t} are not transformable. Try using atomic literals.')
        else:
            raise ValueError(f'No transformer found for type {t}')

    def __init__(self,
                 t: type[T],
                 transformers: list[Transformer[Any]]) -> None:
        self.t = t
        self.type_transformer = {
            atomic_type.t: atomic_type
            for atomic_type in transformers
        }
        self.transformer: LeafTransformer[T] = self._get_transformer(t)
    
    def parse(self, json: JsonObj) -> T:
        return self.transformer.parse(json)
    
    def serialize(self, value: T) -> JsonObj:
        return self.transformer.serialize(value)
