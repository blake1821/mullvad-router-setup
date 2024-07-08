from http.server import BaseHTTPRequestHandler, HTTPServer
import time
from config.config import ConfigManager
from config.config_access import ConfigDAO
from services.services import MULLVAD_SERVICE, MullvadReloadHandler
from web.commands.programs import CommandRoutes
from web.components.template import CenteredPanelTemplate, DefaultPageTemplate
from web.context import HTTPMethod, RequestContext, Page
from web.auth import AuthHandler
from web.pages.homepage import HomepageHandler
from web.route import Router
from web.static import StaticRoutes

def reload():
    MULLVAD_SERVICE.restart()
    time.sleep(1)

config = ConfigDAO(ConfigManager(), MullvadReloadHandler())

file_not_found_page = Page('File not found')

root_handler = Router(file_not_found_page)
static_routes = StaticRoutes(root_handler, 'static')

base_template = DefaultPageTemplate(
    static_routes,
    'Mullvad Router',
)

login_page_template = CenteredPanelTemplate(
    'VPN Router'
)

protected_handler = Router(file_not_found_page)
root_handler.add_route(None, '/',  AuthHandler(
    protected_handler,
    config.login_cookie,
    config.router_password,
    base_template.compose(login_page_template),
    file_not_found_page
))

command_routes = CommandRoutes(protected_handler, '/')
protected_handler.add_route('GET', '/', HomepageHandler(
    base_template,
    'Mullvad Router',
    command_routes
))

class RouterWebsite(BaseHTTPRequestHandler):

    def _handle(self, method: HTTPMethod) -> None:
        ctx = RequestContext(
            self,
            method,
            config
        )
        response = root_handler.handle(ctx)
        ctx.send_response(response)

    def do_POST(self):
        self._handle('POST')

    def do_GET(self):
        self._handle('GET')
    

HTTPServer(('', config.admin_port), RouterWebsite).serve_forever()
