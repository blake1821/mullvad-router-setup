from typing import Generic, Optional, TypeVar
from web.commands.program import Command, Program
from web.context import RequestContext, HTTPMethod, NotFoundResponse, OkBytesResponse, OkPageResponse, Page, RedirectionResponse, Response

class RequestHandler:
    def handle(self,
               context: RequestContext
               ) -> Response:
        raise NotImplementedError()

H = TypeVar('H', bound=RequestHandler)
class Router(RequestHandler):
    """Do not extend"""
    def __init__(self, file_not_found_page: Page):
        self.routes : list[Route[RequestHandler]] = []
        self.file_not_found_page = file_not_found_page
    
    def handle(self, 
               context: RequestContext) -> Response:
        for route in self.routes:
            if (not route.method or context.method == route.method) and context.request.path.startswith(route.path):
                return route.handler.handle(context)
        return NotFoundResponse(self.file_not_found_page)
    
    def add_route(self,
                  method: Optional[HTTPMethod],
                  path: str,
                  handler: H|H):
        route = Route[H](self, method, path, handler)
        self.routes.append(route) # type: ignore
        return route

class Route(Generic[H]):
    def __init__(self,
                 router: Router,
                 method: Optional[HTTPMethod],
                 path: str,
                 handler: H):
        assert path.startswith('/')
        self.router = router
        self.method = method
        self.handler = handler
        self.path = path
    
    def __str__(self) -> str:
        return self.path

AnyRoute = Route[RequestHandler]

P = TypeVar('P', bound=Program) 
class CommandHandler(RequestHandler, Generic[P]):
    def __init__(self, redirect_to: str, program: type[P]) -> None:
        self.program = program
        self.redirect_to = redirect_to
    
    def handle(self, context: RequestContext) -> Response:
        if context.method == 'POST':
            command = Command(self.program)
            command.load_args(context.get_body_args(), context.config)
            command.execute(context)
            response = RedirectionResponse(self.redirect_to)
            return response
        else:
            return NotFoundResponse(Page(f'Cannot {context.method}'))

class StaticFileHandler(RequestHandler):
    def __init__(self, file_path: str):
        self.file_path = file_path
    
    def handle(self, context: RequestContext) -> Response:
        with open(self.file_path, 'rb') as f:
            return OkBytesResponse(f.read())

class StaticPageHandler(RequestHandler):
    def __init__(self, page: Page):
        self.page = page
    
    def handle(self, context: RequestContext) -> Response:
        return OkPageResponse(self.page)


CommandRoute = Route[CommandHandler[P]]
FileRoute = Route[StaticFileHandler]

