from data.mullvad import MullvadDevice
from data.network import PortForward
from data.vultr import VultrInstance
from config.data_access import ConfigAccessor
from services.services import DHCP_SERVICE
from util import bash_get
from web.commands.program import Command
from web.commands.programs import CommandRoutes
from data.vpn import VPN
from web.context import RequestContext, OkPageResponse, Response
from web.form import HTMLTableForm, SubmitButtonForm
from web.pages.html_util import get_html_table, to_html
from web.route import RequestHandler
from web.pages.template import PageTemplate


# Inline strings

def get_connected_message() -> str:
    try:
        endpoint_ip=bash_get("wg | grep 'endpoint:' | awk '{ print $2 }' | cut -d ':' -f 1")
        if endpoint_ip:
            return f"{to_html('✅')} Connected to <span class=\"code\">{endpoint_ip}</span>"
    except:
        pass
    return f"{to_html('❌')} Not connected"


# Buttons

def get_set_vpn_button(vpn: VPN, command_routes: CommandRoutes):
    route = command_routes.set_vpn
    command = Command(route.handler.program) 
    command.args.vpn = vpn
    return SubmitButtonForm(route, 'Set VPN', command=command).html

def get_disable_vpn_button(command_routes: CommandRoutes):
    route = command_routes.set_vpn
    command = Command(route.handler.program) 
    command.args.vpn = None
    return SubmitButtonForm(route, 'Disable', command=command).html

def get_forget_mullvad_device_button(device: MullvadDevice, command_routes: CommandRoutes):
    route = command_routes.mullvad_forget
    command = Command(route.handler.program)
    command.args.device = device
    return SubmitButtonForm(route, 'Forget', command=command).html

def get_destroy_vultr_instance_button(instance: VultrInstance, command_routes: CommandRoutes):
    route = command_routes.vultr_destroy
    command = Command(route.handler.program)
    command.args.instance = instance
    return SubmitButtonForm(route, 'Destroy', command=command).html

def get_delete_port_forward_button(port_fw: PortForward, command_routes: CommandRoutes):
    route = command_routes.delete_port_fw
    command = Command(route.handler.program)
    command.args.src_port = port_fw.src_port
    return SubmitButtonForm(route, 'Delete', command=command).html

# Tables

def get_mullvad_devices_table(config: ConfigAccessor, command_routes: CommandRoutes):
    return get_html_table([
        [
            device.location.name,
            get_set_vpn_button(device, command_routes),
            get_forget_mullvad_device_button(device, command_routes)
        ]
        for device in config.get_mullvad_devices()
    ])


def get_vultr_instances_table(config: ConfigAccessor, command_routes: CommandRoutes):
    return get_html_table([
        [
            str(host.ip),
            # let's skip the details button for now
            get_set_vpn_button(instance, command_routes),
            get_destroy_vultr_instance_button(instance, command_routes),
        ] if host else [
            "Loading...",
            # "",
            "",
            ""
        ]
        for instance in config.get_vultr_instances()
        for host in [config.get_vultr_instance_host(instance.id)]
    ])

def get_client_table() -> str:
    return get_html_table([
        [
            client.mac_address,
            str(client.ip_address),
            client.hostname
        ]
        for client in DHCP_SERVICE.get_clients()
    ], headers=["MAC", "IP", "Hostname"])

def get_port_fw_table(config: ConfigAccessor, command_routes: CommandRoutes) -> str:
    return get_html_table([
        [
            str(port_fw.src_port),
            to_html('→'),
            str(port_fw.dst_addr),
            str(port_fw.dst_port),
            get_delete_port_forward_button(port_fw, command_routes)
        ]
        for port_fw in config.get_port_forwards()
    ], ["Source Port", "", "Destination Address", "Destination Port", ""])

# Sections

def get_vultr_section(config: ConfigAccessor, command_routes: CommandRoutes):
    if config.get_vultr_api_key() is not None:
        return f'''
            <h3>Vultr Instances</h3>
            <div class="centered">
                {get_vultr_instances_table(config, command_routes)}
            </div>
            <h3>Create New Instance</h3>
            <div class="centered">
                {HTMLTableForm(command_routes.vultr_add, 'Add').html}
            </div>
            '''
    else:
        return f'''
            <div class="centered">
            <div>
                <p>Link your Vultr account</p>
                {HTMLTableForm(command_routes.vultr_link, 'Link').html}
            </div>
            </div>
        '''


class HomepageHandler(RequestHandler):
    def __init__(self, template: PageTemplate, title: str, command_routes: CommandRoutes):
        self.template = template
        self.title = title
        self.command_routes = command_routes
    
    def handle(self, context: RequestContext) -> Response:
        config = context.config
        command_routes = self.command_routes
        return OkPageResponse(self.template.get_page(f'''
            <body>
                <h1>{self.title}</h1>
                <div class="centered">
                    <div>
                        <p style="margin: 0px;">{get_connected_message()}</p>
                        { (
                            get_disable_vpn_button(command_routes) + ' ' +
                            SubmitButtonForm(command_routes.restart, 'Restart', margin_top=True).html
                        ) if config.get_selected_vpn() else ""}
                        <br/>
                    </div>
                </div>
                <hr/>
                <h3>Mullvad Devices</h3>
                <div class="centered">
                    {get_mullvad_devices_table(config, command_routes)}
                </div>
                <h3>Add a Mullvad Device</h3>
                <p>The public key is <span class="code">{config.private_key.get_public_key()}</code></p>
                <div class="centered">
                    {HTMLTableForm(command_routes.mullvad_add, 'Add').html}
                </div>
                <hr/>
                {get_vultr_section(config, command_routes)}
                <hr/>
                <h2>Client List</h2>
                <div class="centered">
                    {get_client_table()}
                </div>
                <h2>Port Forwards</h2>
                <div class="centered">
                    {get_port_fw_table(config, command_routes)}
                </div>
                <h3>Forward a new port:</h3>
                <div class="centered">
                    {HTMLTableForm(command_routes.add_port_fw, 'Add').html}
                </div>
            </body>
'''))