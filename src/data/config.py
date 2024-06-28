from dataclasses import dataclass

@dataclass
class Config:
    dns: str
    dns6: str
    lan_v4_interface: str
    lan_v6_interface: str
    


