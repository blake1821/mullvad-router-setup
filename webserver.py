#!/bin/python3
import os
from http.server import *
from urllib.parse import urlparse, parse_qs
import subprocess

os.chdir('/root/vpn')

def launch(program, *args):
    return subprocess.run([program, *args], stdout=subprocess.PIPE).stdout

class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        query = parsed_path.query

        if path.startswith('/add-port-fw'):
            args = parse_qs(query)
            response = launch('./add-port-fw.sh', args['sp'][0], args['da'][0], args['dp'][0])
        elif path.startswith('/restart'):
            response = launch('./restart.sh')
        else:
            response = launch('./homepage.sh')

        if response.decode('utf-8').startswith('redirect'):
            self.send_response(302)
            self.send_header('Location', '/')
            self.end_headers()
        else:
            self.send_response(200)
            self.end_headers()
            self.wfile.write(response)

HTTPServer(('', 80), RequestHandler).serve_forever()
