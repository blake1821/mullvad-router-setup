from config.config_access import ConfigDAO
from services.service import Service
import re

from util.bash import bash

class Grub(Service):
    def __init__(self) :
        super().__init__(
            apt_packages=[],
            service_name=None,
            config_path='/etc/default/grub'
        )

    def get_config_string(self, config: ConfigDAO) -> str:
        return re.sub(
            r'^GRUB_TIMEOUT=.*$',
            'GRUB_TIMEOUT=0',
            self.get_config_file_contents(),
            flags=re.MULTILINE
        )
    
    def _post_config(self):
        bash('update-grub')
    
