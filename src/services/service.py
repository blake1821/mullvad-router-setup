
from typing import Optional

from config.data_access import ConfigAccessor
from util import bash, write_file

class Service:
    apt_packages: list[str]
    service_name: Optional[str]
    config_path: Optional[str]

    def __init__(self, *,
                 apt_packages: list[str]=[],
                 service_name: Optional[str]=None,
                 config_path: Optional[str]=None
                ):
        self.apt_packages = apt_packages
        self.service_name = service_name
        self.config_path = config_path
    
    def get_config_string(self, config: ConfigAccessor) -> str:
        raise NotImplementedError()
    
    def enable(self):
        if self.service_name:
            bash(f'systemctl enable {self.service_name}')

    def restart(self):
        if self.service_name:
            bash(f'systemctl restart {self.service_name}')

    def stop(self):
        if self.service_name:
            bash(f'systemctl stop {self.service_name}')
    
    def write_config_file(self, config: ConfigAccessor):
        if self.config_path:
            write_file(self.config_path, self.get_config_string(config)) 


        