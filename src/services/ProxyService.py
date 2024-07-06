from config.config_access import ConfigAccessor
from services.service import Service


class ProxyService(Service):
    def __init__(self):
        super().__init__(
            apt_packages=['dante-server'],
            service_name='danted',
            config_path='/etc/danted.conf'
        )

    def get_config_string(self, config: ConfigAccessor) -> str:
        return f'''\
logoutput: syslog
user.privileged: root
user.unprivileged: nobody

# The listening network interface or address.
internal: {config.man_ifname} port={config.proxy_port}

# The proxying network interface or address.
external: {config.wg_ifname}

# socks-rules determine what is proxied through the external interface.
clientmethod: none
socksmethod: none

client pass {{
    from: 0/0 to: 0/0
}}

socks pass {{
    from: 0/0 to: 0/0
}}
'''