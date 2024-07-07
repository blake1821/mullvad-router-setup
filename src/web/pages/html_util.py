from typing import Optional
from html import escape

def render_html_table(rows: list[list[str]], headers:Optional[list[str]] = None) -> str:

    def get_row(row: list[str]) -> str:
        return f'''<tr>
            {"".join((
                f"<td>{col}</td>"
                for col in row
            ))}
        </tr>'''
    
    def get_header_row(labels: list[str]) -> str:
        return f'''<tr>
            {"".join((
                f"<th>{th}</th>"
                for th in labels
            ))}
        </tr>'''

    return f'''
    <table>
        { 
            get_header_row(headers)
            if headers else ""
        }
        {"".join((
            get_row(row)
            for row in rows
        ))}
    </table>'''

def to_html(raw: str) -> str:
    return escape(raw).encode('ascii', 'xmlcharrefreplace').decode('ascii')
