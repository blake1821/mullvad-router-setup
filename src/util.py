# Run a bash command and return its output
from abc import ABC, abstractmethod
from datetime import datetime, timedelta
import os
import subprocess
import random
import string
from typing import Generic, Self, TypeVar

def bash_get(command: str):
    try:
        result = subprocess.run(
            command,
            shell=True,
            executable='/bin/bash',
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        raise e


def bash(command: str):
    try:
        subprocess.run(
            command,
            shell=True,
            executable='/bin/bash',
            check=True
        )
    except subprocess.CalledProcessError as e:
        # Handle the exception if the command fails
        print(f"Command failed with exit code {e.returncode}")
        return False
    return True


def write_file(path: str, contents: str) -> None:
    with open(path, 'w') as file:
        file.write(contents)

CHARACTERS = string.ascii_letters + string.digits
def get_random_string():
    return ''.join(random.choice(CHARACTERS) for _ in range(20))

def get_vpn_home():
    assert os.path.exists('README.md')
    return os.getcwd()

def cookie_expiration_date():
    expiration_date = datetime.now() + timedelta(days=365)
    return expiration_date.strftime('%a, %d-%b-%Y %H:%M:%S GMT')

T = TypeVar('T')
class Id(Generic[T], str):
    pass

class Identifiable(ABC):
    @abstractmethod
    def get_id(self) -> Id[Self]:
        raise NotImplementedError()
    
    def __eq__(self, value: object) -> bool:
        return (
            isinstance(value, type(self)) and
            self.get_id() == value.get_id()
        )

class Describable(Identifiable): 
    @abstractmethod
    def get_description(self) -> str:
        raise NotImplementedError()


I = TypeVar('I', bound=Identifiable)
IdMap = dict[Id[I], I]

def create_id_map(items: list[I]) -> IdMap[I]:
    return {
        item.get_id(): item
        for item in items
    }
