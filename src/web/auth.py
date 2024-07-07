from web.commands.program import Program
from web.commands.param_type import PASSWORD_PARAM_TYPE
from web.components.template import Template
from web.context import RequestContext, Page, Response
from web.form import HTMLTableFormBuilder
from web.route import CommandHandler, RequestHandler, Router, StaticPageHandler



class AuthHandler(RequestHandler):
    def __init__(self,
                 protected_handler: RequestHandler,
                 login_cookie: str,
                 login_password: str,
                 page_template: Template[Page, str],
                 file_not_found_page: Page):
        self.login_router = Router(file_not_found_page)
        self.protected_handler = protected_handler
        self.login_cookie = login_cookie

        class LoginProgram(Program):
            password = PASSWORD_PARAM_TYPE.param("Password")
            def execute(self, context: RequestContext) -> None:
                if self.password == login_password:
                    context.set_cookie('key', login_cookie)

        login_route = self.login_router.add_route(
            'POST', '/login', CommandHandler('/', LoginProgram)
        )

        login_page = page_template.render(
            HTMLTableFormBuilder('Log In').build(login_route)
        )

        self.login_router.add_route(
            None, '/', StaticPageHandler(login_page)
        )


    def handle(self, context: RequestContext) -> Response:
        # get the login cookie
        cookies = context.get_cookies()
        login_cookie: str = ""
        if 'key' in cookies:
            login_cookie = cookies['key'].value

        if login_cookie == self.login_cookie:
            return self.protected_handler.handle(context)
        else:
            return self.login_router.handle(context)