from typing import TypeVar
from web.commands.delete_port_forward import DeletePortForwardProgram
from web.commands.mullvad_add import AddMullvadDeviceProgram
from web.commands.add_port_forward import AddPortForwardProgram
from web.commands.program import Program
from web.commands.mullvad_forget import ForgetMullvadDeviceProgram
from web.commands.restart import RestartProgram
from web.commands.set_vpn import SetVPNProgram
from web.commands.vultr_add import AddVultrInstanceProgram
from web.commands.vultr_destroy import DestroyVultrInstanceProgram
from web.commands.vultr_link import LinkVultrProgram
from web.route import CommandHandler, Router

class CommandRoutes:
    def __init__(self, router: Router, homepage_path: str):
        P = TypeVar('P', bound=Program)
        def add_route(name: str, command: type[P]):
            return router.add_route('POST', name, CommandHandler(homepage_path, command))

        self.add_port_fw = add_route('/add-port-fw', AddPortForwardProgram)
        self.delete_port_fw = add_route('/delete-port-fw', DeletePortForwardProgram)
        self.restart = add_route('/restart', RestartProgram)
        self.set_vpn = add_route('/set-vpn', SetVPNProgram)
        self.mullvad_forget = add_route('/mullvad-forget', ForgetMullvadDeviceProgram)
        self.mullvad_add = add_route('/mullvad-add', AddMullvadDeviceProgram)
        self.vultr_link = add_route('/vultr-link', LinkVultrProgram)
        self.vultr_add = add_route('/vultr-add', AddVultrInstanceProgram)
        self.vultr_destroy = add_route('/vultr-destroy', DestroyVultrInstanceProgram)
        
