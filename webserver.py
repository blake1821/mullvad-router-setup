#!/bin/python3
import os
from http.server import *
from http.cookies import *
from urllib.parse import urlparse, parse_qs
import subprocess
from datetime import datetime, timedelta

os.chdir(os.path.realpath(os.path.dirname(__file__)))

def launch(program, *args):
    return subprocess.run([program, *args], stdout=subprocess.PIPE).stdout

def cookie_expiration_date():
    expiration_date = datetime.now() + timedelta(days=365)
    return expiration_date.strftime('%a, %d-%b-%Y %H:%M:%S GMT')

# Requests need this as their 'key' cookie to log in
with open('const/login-cookie') as login_cookie_file:
    LOGIN_COOKIE = login_cookie_file.read().strip()

with open('const/psw') as password_file:
    password = password_file.read().strip()


class RequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_len = int(self.headers.get('Content-Length', 0))
        body = ""
        if content_len:
            body = self.rfile.read(content_len).decode('utf-8')

        self.do_GET(body)

    def do_GET(self, body=""):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        query = parsed_path.query
        args = parse_qs(query)

        # get the login cookie
        cookies_string = self.headers.get('Cookie')
        login_cookie = ""
        if cookies_string:
            cookies = SimpleCookie()
            cookies.load(cookies_string)
            if 'key' in cookies:
                login_cookie = cookies['key'].value

        if path.startswith('/style.css'):
            with open('./style.css', 'rb') as stylesheet:
                response = stylesheet.read()
        elif login_cookie != LOGIN_COOKIE:
            body_args = parse_qs(body)
            if 'password' in body_args and body_args['password'][0] == password:
                response = 'authenticate'
            else:
                with open('login.html', 'rb') as login_page:
                    response = login_page.read()
        else:
            if path.startswith('/add-port-fw'):
                response = launch('./add-port-fw.sh', args['sp'][0], args['da'][0], args['dp'][0])
            elif path.startswith('/restart'):
                response = launch('./restart.sh')
            elif path.startswith('/configure'):
                ep_type = args['endpoint-type'][0]
                my4 = args['my4'][0]
                my6 = args['my6'][0]
                if ep_type == 'mullvad':
                    location = args['location'][0]
                    response = launch('./configure.sh', 'mullvad', my4, my6, location)
                else:
                    endpoint_ip = args['endpoint-ip'][0]
                    endpoint_pubkey = args['endpoint-pubkey'][0]
                    response = launch('./configure.sh', 'custom', my4, my6, endpoint_ip, endpoint_pubkey)
            else:
                response = launch('./homepage.sh')

        if response == 'authenticate':
            self.send_response(302)
            self.send_header('Location', '/')
            expires=cookie_expiration_date()
            self.send_header('Set-Cookie', f'key={LOGIN_COOKIE}; SameSite=Strict; Expires={expires}')
            self.end_headers()
        elif response.decode('utf-8').startswith('redirect'):
            self.send_response(302)
            self.send_header('Location', '/')
            self.end_headers()
        else:
            self.send_response(200)
            self.end_headers()
            self.wfile.write(response)

HTTPServer(('', 80), RequestHandler).serve_forever()
