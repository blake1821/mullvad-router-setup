from abc import ABC, abstractmethod
from typing import Generic, Iterable, TypeVar

from web.context import Page
from web.static import StaticRoutes

O = TypeVar('O')
M = TypeVar('M')
I = TypeVar('I')
class TemplateBase(Generic[O, I], ABC):
    @abstractmethod
    def render(self, insides: I) -> O:
        raise NotImplementedError()

class Template(TemplateBase[O, M]):
    def compose(self, inner: TemplateBase[M, I]):
        return CompositionalTemplate(self, inner)

class ArrayTemplate(Template[O, Iterable[I]]):
    def __init__(self, outer: TemplateBase[O, str], inner: TemplateBase[str, I]):
        self.outer = outer
        self.inner = inner
    
    def render(self, insides: Iterable[I]) -> O:
        return self.outer.render(''.join(self.inner.render(i) for i in insides))

class CompositionalTemplate(Template[O, I]):
    def __init__(self, outer: TemplateBase[O, M], inner: TemplateBase[M, I]):
        self.outer = outer
        self.inner = inner
    
    def render(self, insides: I) -> O:
        return self.outer.render(self.inner.render(insides))
       

class DefaultPageTemplate(Template[Page, str]):
    def __init__(self, static_routes: StaticRoutes, title: str):
        self.static_routes = static_routes
        self.title = title

    def render(self, insides: str):
        return Page(f'''
        <html>
            <head>
                <title>{self.title}</title>
                <link rel="stylesheet" href="{self.static_routes.stylesheet}">
            </head>
            <body>
                {insides}
            </body>
        </html>
        ''')

class CenteredPanelTemplate(Template[str, str]):
    def __init__(self, title: str):
        self.title = title

    def render(self, insides: str):
        return f'''
        <div class="centered-panel-container">
            <div>
                <h2>{self.title}</h2>
                {insides}
            </div>
        </div>
        '''

class SectionsTemplate(Template[str, list[str]]):
    def render(self, insides: list[str]):
        return '<div class="sections-container">' + \
            ''.join(f'<section>{inside}</section>' for inside in insides) + \
        '</div>'


    