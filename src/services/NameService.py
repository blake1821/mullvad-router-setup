from config.config_access import ConfigAccessor
from services.service import Service


class NameService(Service):
    def __init__(self):
        super().__init__(
            config_path='/etc/resolv.conf'
        )

    def get_config_string(self, config: ConfigAccessor) -> str:
        return f'''
nameserver {config.dns}
nameserver {config.dns6}
'''