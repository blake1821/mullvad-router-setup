from abc import ABC, abstractmethod
from dataclasses import fields, is_dataclass
from typing import Any, Generic, Literal, Optional, Protocol, TypeVar, TypedDict, cast, get_type_hints

from util.typing import BaseType, ClassTypeDescription, JsonAtom, LiteralTypeDescription, TypeDescription, TypedDictDescription, get_type_description
from util.typing import JsonObj

# Type Variables
T = TypeVar('T')
JT = TypeVar('JT', bound=int | str | bool | None.__class__)


# Base classes

class Transformer(ABC, Generic[T]):

    @abstractmethod
    def parse(self, json: JsonObj) -> T:
        raise NotImplementedError()

    @abstractmethod
    def serialize(self, value: T) -> JsonObj:
        raise NotImplementedError()

class BaseTransformerRegistry(ABC):
    @abstractmethod
    def get_transformer(self, t: type[T]) -> Transformer[T]:
        raise NotImplementedError()

class TransformerFactory(Protocol[T]):
    """
    A transformer that may be composed of other transformers.
    Use the abstract factory method `create` to instantiate the transformer for varying types.
    Often, the type arguments in the type description are used to determine the inner transformers.
    """

    def __call__(self, type_description: TypeDescription[T], transformer_registry: BaseTransformerRegistry) -> Transformer[T]:
        raise NotImplementedError()

# Atom transfomers

class _JsonAtomTransformer(Transformer[JsonAtom]):
    """Transformer for atomic types like int, str, bool, and None."""

    def parse(self, json: JsonObj) -> JsonAtom:
        assert isinstance(json, JsonAtom)
        return json

    def serialize(self, value: JsonAtom) -> JsonObj:
        return value
    
    @classmethod
    def create(cls, type_description: TypeDescription[int | str | bool | None], transformer_registry: BaseTransformerRegistry) -> Transformer[JsonAtom]:
        return _JsonAtomTransformer()

"""A protocol for any class that can be constructed from its string representation, such as IPv4Address."""
U = TypeVar('U', covariant=True)
class StringInit(Protocol[U]):
    def __call__(self, *args: *tuple[str]) -> U:
        ...

class _StringInitTransformer(Transformer[T]): 
    """Transforms a class with a string constructor to and from a string.
    Note: This class doesn't perform adequate type checking. Use it carefully."""

    def __init__(self, t: StringInit[T]):
        self.t = t

    def parse(self, json: JsonObj) -> T:
        assert isinstance(json, str)
        return self.t(json)
    
    def serialize(self, value: T) -> JsonObj:
        return str(value)

    @classmethod
    def create(cls, type_description: TypeDescription[T], transformer_registry: BaseTransformerRegistry) -> Transformer[T]:
        assert isinstance(type_description, ClassTypeDescription)
        base_type = cast(type[T], type_description.base_type)
        return _StringInitTransformer(base_type)

# Composite transformers

class _TypedDictTransformer(Transformer[T]):
    """Transforms a TypedDict using its type annotations."""

    def __init__(self,
                 typed_dict_type: type[T],
                 transformer_registry: BaseTransformerRegistry):
        self.attribute_transformers = {
            k: transformer_registry.get_transformer(at)
            for k, at in get_type_hints(typed_dict_type).items()
        }

    @classmethod
    def create(cls, type_description: TypeDescription[T], transformer_registry: BaseTransformerRegistry) -> Transformer[T]:
        assert isinstance(type_description, TypedDictDescription)
        return _TypedDictTransformer(type_description.typed_dict_type, transformer_registry)
    
    @classmethod
    def get_base_type(cls) -> BaseType:
        return 'TypedDict'

    def parse(self, json: JsonObj) -> T:
        assert isinstance(json, dict)
        return cast(T, {
            k: at.parse(json[k])
            for k, at in self.attribute_transformers.items()
        })
    
    def serialize(self, value: T) -> JsonObj:
        casted_value = cast(dict[str, Any], value)
        return {
            k: at.serialize(casted_value[k])
            for k, at in self.attribute_transformers.items()
        }


class _ListTransformer(Transformer[list[T]]):
    """Transforms a list[T] with an annotated value type"""
    def __init__(self,
                 attribute_transformer: Transformer[T]):
        self.attribute_transformer = attribute_transformer

    @classmethod
    def create(cls, type_description: TypeDescription[list[T]], transformer_registry: BaseTransformerRegistry) -> Transformer[list[T]]:
        assert isinstance(type_description, ClassTypeDescription)
        return _ListTransformer(
            transformer_registry.get_transformer(type_description.type_args[0])
        )
    
    @classmethod
    def get_base_type(cls) -> BaseType:
        return list

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

class _DictTransformer(Transformer[dict[str, T]]):
    """Transforms a dict[str, T] with an annotated value type"""

    def __init__(self,
                 attribute_transormer: Transformer[T]):
        self.attribute_transformer = attribute_transormer
    
    @classmethod
    def create(cls, type_description: TypeDescription[dict[str, T]], transformer_registry: BaseTransformerRegistry) -> Transformer[dict[str, T]]:
        assert isinstance(type_description, ClassTypeDescription)
        return _DictTransformer(
            transformer_registry.get_transformer(type_description.type_args[1])
        )
    
    @classmethod
    def get_base_type(cls) -> BaseType:
        return dict

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

class _OptionalTransformer(Transformer[Optional[T]]):
    """Transforms an Optional[...] value"""

    def __init__(self,
                 attribute_transormer: Transformer[T]):
        self.attribute_transformer = attribute_transormer

    @classmethod
    def create(cls, type_description: TypeDescription[Optional[T]], transformer_registry: BaseTransformerRegistry) -> Transformer[T | None]:
        assert isinstance(type_description, ClassTypeDescription)
        return _OptionalTransformer(transformer_registry.get_transformer(type_description.type_args[0]))
    
    @classmethod
    def get_base_type(cls) -> Literal['Optional']:
        return 'Optional'

    def parse(self, json: JsonObj) -> Optional[T]:
        if json is None:
            return None
        return self.attribute_transformer.parse(json)
    
    def serialize(self, value: Optional[T]) -> JsonObj:
        if value is None:
            return None
        return self.attribute_transformer.serialize(value)

class _LiteralTransformer(Transformer[JsonAtom]):
    """Transforms a literal value"""

    @classmethod
    def create(cls, type_description: TypeDescription[Any], transformer_registry: BaseTransformerRegistry) -> Transformer[JsonAtom]:
        assert isinstance(type_description, LiteralTypeDescription)
        assert all(isinstance(arg, JsonAtom) for arg in type_description.literal_options)
        return _JsonAtomTransformer()
    
    @classmethod
    def get_base_type(cls) -> Literal['Literal']:
        return 'Literal'


class _DataclassTransformer(Transformer[T]):
    """Transforms between a dataclass and a TypedDict. You can use this when each attribute of the dataclass has a corresponding transformer"""

    def __init__(self, t: type[T], transformer_registry: BaseTransformerRegistry):
        self.t = t
        assert is_dataclass(t)

        self.datadict = TypedDict('datadict', {
            field.name: field.type
            for field in fields(t) # type: ignore
        })

        self.inner_transformer = transformer_registry.get_transformer(self.datadict)
    
    @classmethod
    def create(cls, type_description: TypeDescription[T], transformer_registry: BaseTransformerRegistry) -> Transformer[T]:
        assert isinstance(type_description, ClassTypeDescription)
        dataclass_type = cast(type[T], type_description.base_type)
        return _DataclassTransformer(dataclass_type, transformer_registry)
    
    def parse(self, json: JsonObj) -> T:
        return self.t(**self.inner_transformer.parse(json))
    
    def serialize(self, value: T) -> JsonObj:
        return self.inner_transformer.serialize(self.datadict({
            field.name: vars(value)[field.name]
            for field in fields(self.t) #type: ignore
        }))

class TransformerRegistry(BaseTransformerRegistry):
    def __init__(self) -> None:
        # Base type -> Factory
        self._transformer_factories: dict[BaseType, TransformerFactory[Any]] = {
            **{
                atomic_type: _JsonAtomTransformer.create
                for atomic_type in [int, str, bool, None.__class__]
            },
            'TypedDict': _TypedDictTransformer[Any].create,
            list: _ListTransformer[Any].create,
            dict: _DictTransformer[Any].create,
            'Optional': _OptionalTransformer[Any].create,
            'Literal': _LiteralTransformer.create
        }

    def get_transformer(self, t: type[T]) -> Transformer[T]:
        type_desc = get_type_description(t)
        base_type = type_desc.base_type
        if base_type in self._transformer_factories:
            return self._transformer_factories[base_type](type_desc, self)
        raise ValueError(f'No transformer found for type {t}')
    
    def register_stringlike(self, t: StringInit[T]):
        """Add a transformer for a class that can be constructed from a string."""
        t_type = cast(type[T], t)
        self.register_transformer(t_type, _StringInitTransformer[Any].create)
        return self
    
    def register_transformer(self, t: type[T], factory: TransformerFactory[T]):
        """Add a transformer for a composite type."""
        type_description = get_type_description(t)
        self._transformer_factories[type_description.base_type] = factory
        return self

    def register_dataclass(self, t: type[T]):
        """Add a transformer for a dataclass."""
        self.register_transformer(t, _DataclassTransformer[Any].create)
        return self


# Might be useful later:

# def CustomConfigObject(transformer: LeafTransformer[T]) -> type[T]:
#     return cast(type[T], transformer)

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