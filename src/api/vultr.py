from ipaddress import IPv4Address
from typing import Any, Optional
import requests
from data.vultr import VultrInstance, VultrLocation, VultrPlan
from data.vultr import VultrOS
from data.vultr import ApiKey
from util import Id

class VultrInstanceResponse:
    def __init__(self,
                 id: Id[VultrInstance],
                 password: Optional[str],
                 main_ip: Optional[IPv4Address],
                 status: str):
        self.id = id
        self.password = password
        self.main_ip = main_ip
        self.status = status

def parse_fresh_vultr_instance(response: Any):
    return VultrInstanceResponse(
        id=Id(response['id']),
        password=response['default_password'] if 'default_password' in response else None,
        main_ip=response['main_ip'] if 'main_ip' in response else None,
        status=response['status']
    )


class VultrAPI:
    def __init__(self, api_key: ApiKey):
        self.api_key = api_key
    
    def create_instance(self,
                        plan: VultrPlan,
                        location: VultrLocation,
                        os: VultrOS):
        response = requests.post(
            'https://api.vultr.com/v2/instances',
            json={
                "region": location.id,
                "plan": plan.id,
                "os_id": os.id,
                "enable_ipv6": True,
                "disable_public_ipv4": False
            },
            headers={
                'Authorization': f'Bearer {self.api_key}'
            }
        ).json()['instance']

        return parse_fresh_vultr_instance(response)
    
    def get_instance(self, id: str):
        response = requests.get(
            f'https://api.vultr.com/v2/instances/{id}',
            headers={
                'Authorization': f'Bearer {self.api_key}'
            }
        ).json()['instance']

        return parse_fresh_vultr_instance(response)
    
    def destroy_instance(self, id: str):
        requests.delete(
            f'https://api.vultr.com/v2/instances/{id}',
            headers={
                'Authorization': f'Bearer {self.api_key}'
            }
        )


    
    
        
