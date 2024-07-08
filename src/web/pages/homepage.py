from data.mullvad import MullvadDevice
from data.network import PortForward
from data.vultr import VultrInstance
from config.config_access import ConfigDAO
from services.services import DHCP_SERVICE
from util.bash import bash_get
from web.commands.program import Command
from web.commands.programs import CommandRoutes
from data.vpn import VPN
from web.components.template import SectionsTemplate, Template
from web.context import Page, RequestContext, OkPageResponse, Response
from web.form import HTMLTableFormBuilder, ButtonFormBuilder
from web.pages.html_util import render_html_table, to_html
from web.route import RequestHandler


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

def render_set_vpn_button(vpn: VPN, command_routes: CommandRoutes):
    route = command_routes.set_vpn
    command = Command(route.handler.program) 
    command.args.vpn = vpn
    return ButtonFormBuilder('Set VPN').build(route, command=command)

def render_disable_vpn_button(command_routes: CommandRoutes):
    route = command_routes.set_vpn
    command = Command(route.handler.program) 
    command.args.vpn = None
    return ButtonFormBuilder('Disable').build(route, command=command)

def render_forget_mullvad_device_button(device: MullvadDevice, command_routes: CommandRoutes):
    route = command_routes.mullvad_forget
    command = Command(route.handler.program)
    command.args.device = device
    return ButtonFormBuilder('Forget').build(route, command=command)

def render_destroy_vultr_instance_button(instance: VultrInstance, command_routes: CommandRoutes):
    route = command_routes.vultr_destroy
    command = Command(route.handler.program)
    command.args.instance = instance
    return ButtonFormBuilder('Destroy').build(route, command=command)

def render_delete_port_forward_button(port_fw: PortForward, command_routes: CommandRoutes):
    route = command_routes.delete_port_fw
    command = Command(route.handler.program)
    command.args.src_port = port_fw.src_port
    return ButtonFormBuilder('Delete').build(route, command=command)

# Tables

def render_mullvad_devices_table(config: ConfigDAO, command_routes: CommandRoutes):
    return render_html_table([
        [
            device.location.name,
            render_set_vpn_button(device, command_routes),
            render_forget_mullvad_device_button(device, command_routes)
        ]
        for device in config.get_mullvad_devices()
    ])


def render_vultr_instances_table(config: ConfigDAO, command_routes: CommandRoutes):
    return render_html_table([
        [
            str(host.ip),
            # let's skip the details button for now
            render_set_vpn_button(instance, command_routes),
            render_destroy_vultr_instance_button(instance, command_routes),
        ] if host else [
            "Loading...",
            # "",
            "",
            ""
        ]
        for instance in config.get_vultr_instances()
        for host in [config.get_vultr_instance_host(instance.id)]
    ])

def render_client_table() -> str:
    return render_html_table([
        [
            client.mac_address,
            str(client.ip_address),
            client.hostname
        ]
        for client in DHCP_SERVICE.get_clients()
    ], headers=["MAC", "IP", "Hostname"])

def render_port_fw_table(config: ConfigDAO, command_routes: CommandRoutes) -> str:
    return render_html_table([
        [
            str(port_fw.src_port),
            to_html('→'),
            str(port_fw.dst_addr),
            str(port_fw.dst_port),
            render_delete_port_forward_button(port_fw, command_routes)
        ]
        for port_fw in config.get_port_forwards()
    ], ["Source Port", "", "Destination Address", "Destination Port", ""])

# Sections

def render_vultr_section(config: ConfigDAO, command_routes: CommandRoutes):
    if config.get_vultr_api_key() is not None:
        return f'''
            <div class="centered">
                {render_vultr_instances_table(config, command_routes)}
            </div>
            <h3>Create New Instance</h3>
            <div class="centered">
                {HTMLTableFormBuilder('Add').build(command_routes.vultr_add)}
            </div>
            '''
    else:
        return f'''
            <div class="centered">
            <div>
                <p>Link your Vultr account</p>
                {HTMLTableFormBuilder('Link').build(command_routes.vultr_link)}
            </div>
            </div>
        '''


class HomepageHandler(RequestHandler):
    def __init__(self, template: Template[Page, str], title: str, command_routes: CommandRoutes):
        self.template = template.compose(SectionsTemplate())
        self.title = title
        self.command_routes = command_routes
    
    def handle(self, context: RequestContext) -> Response:
        config = context.config
        command_routes = self.command_routes
        return OkPageResponse(self.template.render([f'''
            <h2>Router Status</h2>
            <div class="centered">
                <div>
                    <p style="margin: 0px;">{get_connected_message()}</p>
                    { (
                        render_disable_vpn_button(command_routes) + ' ' +
                        ButtonFormBuilder('Restart', margin_top=True).build(command_routes.restart)
                    ) if config.get_selected_vpn() else ""}
                </div>
            </div>''',
            f'''
            <h2>Mullvad Devices</h2>
            <div class="centered">
                {render_mullvad_devices_table(config, command_routes)}
            </div>
            <h3>Add a Mullvad Device</h3>
            <p>The public key is <span class="code">{config.private_key.get_public_key()}</code></p>
            <div class="centered">
                {HTMLTableFormBuilder('Add').build(command_routes.mullvad_add)}
            </div>
            ''',
            f'''
            <h2>Vultr Instances</h2>
            {render_vultr_section(config, command_routes)}
            ''',
            f'''
            <h2>Client List</h2>
            <div class="centered">
                {render_client_table()}
            </div>
            ''',
            f'''
            <h2>Port Forwards</h2>
            <div class="centered">
                {render_port_fw_table(config, command_routes)}
            </div>
            <h3>Forward a new port:</h3>
            <div class="centered">
                {HTMLTableFormBuilder('Add').build(command_routes.add_port_fw)}
            </div>
''']))