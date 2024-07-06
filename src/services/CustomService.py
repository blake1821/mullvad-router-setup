from typing import Literal, Union
from config.config_access import ConfigAccessor
from services.service import Service


class CustomService(Service):
    description: str
    exec_path: str
    restart_mode: str

    def __init__(
        self,
        *,
        name: str,
        description: str,
        exec_path: str,
        restart_mode: Union[Literal['always'], Literal['no']],
    ):
        super().__init__(
            service_name=name,
            config_path=f'/etc/systemd/system/{name}.service'
        )
        self.description = description
        self.exec_path = exec_path
        self.restart_mode = restart_mode
    
    def get_config_string(self, config: ConfigAccessor) -> str:
        return f'''\
[Unit]
Description={self.description}

[Service]
Type=simple
ExecStart={self.exec_path}
Restart={self.restart_mode}
After=network.target

[Install]
WantedBy=multi-user.target
'''
    