
from abc import ABC, abstractmethod

from web.context import Page
from web.route import FileRoute

class PageTemplate(ABC):
    @abstractmethod
    def get_page(self, inside: str) -> Page:
        pass

class RouterBaseTemplate(PageTemplate):
    def __init__(self, stylesheet: FileRoute):
        self.stylesheet = stylesheet

    def get_page(self, inside: str):
        return Page(f'''
        <html>
            <head>
                <title>Mullvad Router</title>
                <link rel="stylesheet" href="{self.stylesheet.path}">
            </head>
            {inside}
        </html>
        ''')

class RouterCenteredTemplate(RouterBaseTemplate):
    def __init__(self, stylesheet: FileRoute, page_title: str):
        super().__init__(stylesheet)
        self.page_title = page_title

    def get_page(self, inside: str):
        return super().get_page(f'''
        <body id="login-screen" class="centered">
            <div>
                <h2>{self.page_title}</h2>
                {inside}
            </div>
        </body>
        ''')