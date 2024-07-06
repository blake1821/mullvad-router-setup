from dataclasses import dataclass
from ipaddress import IPv4Address, IPv6Address
import json
from typing import Literal, Optional, TypedDict
from config.transformer import ATOM_TRANSFORMERS, DataclassTransformer, JsonObj, NestedTransfomer, StringInitTransformer
from config.transformer import LeafTransformer
from util import Id

class DummyChild(TypedDict):
    a: int
    b: str
    c: bool
    ipv4: IPv4Address
    ipv6: IPv6Address

class DummyParent(TypedDict):
    ch: DummyChild
    cho: Optional[DummyChild]


class IPTransformer(LeafTransformer[IPv4Address | IPv6Address]):
    def parse(self, json: JsonObj) -> IPv4Address | IPv6Address:
        assert isinstance(json, str)
        return self.t(json)
    
    def serialize(self, value: IPv4Address | IPv6Address) -> JsonObj:
        return str(value)

try:
    trans = NestedTransfomer(
        DummyParent,
        [*ATOM_TRANSFORMERS, IPTransformer(IPv4Address)]
    )

    ser = trans.serialize({
        'ch': {
            'a': 1,
            'b': 'hi',
            'c': True,
            'ipv4': IPv4Address('1.1.1.1'),
            'ipv6': IPv6Address('::1')
        },
        'cho': None
    })
    assert False
except ValueError as e:
    assert 'IPv6' in str(e)
print("passed 1")

trans = NestedTransfomer(
    DummyParent,
    [*ATOM_TRANSFORMERS, IPTransformer(IPv4Address), StringInitTransformer(IPv6Address)]
)

ser = trans.serialize({
    'ch': {
        'a': 1,
        'b': 'hi',
        'c': True,
        'ipv4': IPv4Address('1.1.1.1'),
        'ipv6': IPv6Address('::1')
    },
    'cho': None
})

expected = {
    'ch': {
        'a': 1,
        'b': 'hi',
        'c': True,
        'ipv4': '1.1.1.1',
        'ipv6': '::1'
    },
    'cho': None
}

assert json.dumps(ser) == json.dumps(expected)
print('passed 2')

ser = trans.serialize({
    'ch': {
        'a': 1,
        'b': 'hi',
        'c': True,
        'ipv4': IPv4Address('1.1.1.1'),
        'ipv6': IPv6Address('::1')
    },
    'cho': {
        'a': 2,
        'b': 'bye',
        'c': False,
        'ipv4': IPv4Address('1.2.2.1'),
        'ipv6': IPv6Address('::2')
    }
})

expected = {
    'ch': {
        'a': 1,
        'b': 'hi',
        'c': True,
        'ipv4': '1.1.1.1',
        'ipv6': '::1'
    },
    'cho': {
        'a': 2,
        'b': 'bye',
        'c': False,
        'ipv4': '1.2.2.1',
        'ipv6': '::2'
    }
}

assert json.dumps(ser) == json.dumps(expected)
print('passed 3')

class LiteralContainer(TypedDict):
    xd: Literal['x', 'd']

literal_transformer = NestedTransfomer(LiteralContainer, [*ATOM_TRANSFORMERS])

result = json.dumps(literal_transformer.serialize({'xd': 'x'}))
assert result == '{"xd": "x"}'
print('passed 4')

class GenericContainer(TypedDict):
    id: Id[str]

generic_transformer = NestedTransfomer(GenericContainer, [*ATOM_TRANSFORMERS, StringInitTransformer(Id)])
result = json.dumps(generic_transformer.serialize({'id': Id('fsfsd')}))
assert result == '{"id": "fsfsd"}'
print('passed 5')


@dataclass
class Dog:
    bones: int
dog_transformer = NestedTransfomer(Dog, [
    *ATOM_TRANSFORMERS,
    DataclassTransformer(Dog)
])
result = json.dumps(dog_transformer.serialize(Dog(bones=4)))
assert result == '{"bones": 4}'
print('passed 6')


from config.config import ConfigManager
config_manager = ConfigManager()
x = config_manager.load()

print(x)