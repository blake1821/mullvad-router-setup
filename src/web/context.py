from http.cookies import Morsel, SimpleCookie
from typing import Literal, Mapping, Optional, Self
from config.config_access import ConfigDAO
from util.util import cookie_expiration_date
from http.server import BaseHTTPRequestHandler
from urllib.parse import parse_qs


HTTPMethod = Literal['GET', 'POST']

class BaseResponse():
    def __init__(self, response_code: int, body: Optional[bytes]):
        self.response_code = response_code
        self.body = body
        self.headers : list[tuple[str, str]] = []
    
class Page:
    def __init__(self, html: str):
        self.html = html

class RedirectionResponse(BaseResponse):
    def __init__(self, redirect_to: str):
        super().__init__(302, None)
        self.headers.append(('Location', redirect_to))

class OkPageResponse(BaseResponse):
    def __init__(self, page: Page):
        super().__init__(200, page.html.encode('utf-8'))
        self.headers.append(('Content-Type', 'text/html'))

class NotFoundResponse(BaseResponse):
    def __init__(self, page: Page):
        super().__init__(404, page.html.encode('utf-8'))

class OkBytesResponse(BaseResponse):
    def __init__(self, body: bytes):
        super().__init__(200, body) 

Response = RedirectionResponse | OkPageResponse | OkBytesResponse | NotFoundResponse

class RequestContext():

    def __init__(self,
                 request: BaseHTTPRequestHandler,
                 method: HTTPMethod,
                 data_accessor: ConfigDAO) -> None:
        self.request = request
        self.method = method
        self.config = data_accessor
        self.optional_response_headers : list[tuple[str, str]] = []
        self.has_sent = False

    def get_body_args(self):
        content_len = int(self.request.headers.get('Content-Length', 0))
        body = ""
        if content_len:
            body = self.request.rfile.read(content_len).decode('utf-8')
        return {
            k: v[0]
            for [k, v] in parse_qs(body).items()
        }
    
    def get_cookies(self) -> Mapping[str, Morsel[str]]:
        cookies_string = self.request.headers.get('Cookie')
        if cookies_string:
            cookies = SimpleCookie()
            cookies.load(cookies_string)
            return cookies
        return {}
    
    def set_cookie(self, name: str, value: str) -> Self:
        expires=cookie_expiration_date()
        self.optional_response_headers.append(('Set-Cookie', f'{name}={value}; SameSite=Strict; Expires={expires}'))
        return self
    
    def send_response(self, response: Response) -> None:
        if self.has_sent:
            raise RuntimeError('Already sent response!')
        self.has_sent = True

        self.request.send_response(response.response_code)
        for name, value in [*response.headers, *self.optional_response_headers]:
            self.request.send_header(name, value)
        body = response.body
        if body:
            content_length = len(body)
            self.request.send_header('Content-Length', str(content_length))
            self.request.end_headers()
            self.request.wfile.write(body)
        else:
            self.request.end_headers()



    
