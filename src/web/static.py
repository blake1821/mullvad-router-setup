from os import path
from web.route import Router, StaticFileHandler


class StaticRoutes:
    def __init__(self, router: Router, static_path: str):
        def add_route(filename: str):
            assert not filename.startswith('/')
            file_path = path.join(static_path, filename)
            assert path.exists(file_path)
            return router.add_route('GET', '/'+filename, StaticFileHandler(file_path))

        self.stylesheet = add_route('style.css')
        self.puro = add_route('puro-bg.png')