from time import sleep
from data.network import PrivateKey
from data.vultr import VULTR_LOCATIONS, VULTR_PLANS, VultrInstance, VultrInstanceHost
from config.config_access import ConfigAccessor
from network.nc import is_tcp_port_open
from util import bash
from api.vultr import VultrInstanceResponse, VultrAPI
from data.vultr import VULTR_DEBIAN_12
from web.commands.param_type import SelectParamType
from web.commands.program import Program
from web.context import RequestContext
import threading

def load_vultr_instance(
        config: ConfigAccessor,
        vultr_api: VultrAPI,
        initial_instance: VultrInstanceResponse,
        instance_private_key: str):
    instance = initial_instance
    instnace_id = initial_instance.id
    password = initial_instance.password
    assert password

    while instance.status != 'active':
        sleep(10)
        instance = vultr_api.get_instance(instnace_id)
    
    instance_ipv4 = instance.main_ip
    assert instance_ipv4

    while not is_tcp_port_open(instance_ipv4, 22):
        sleep(5)
    
    bash(f'''
        sshpass -p {password} scp -o StrictHostKeyChecking=no ./vultr-setup.sh root@{instance_ipv4}:/root/setup.sh
        sshpass -p {password} ssh -o StrictHostKeyChecking=no root@{instance_ipv4} <<EOF
            chmod +x /root/setup.sh
            /root/setup.sh {instance_private_key} {config.private_key.get_public_key()}
EOF
    ''')
    config.set_vultr_instance_host(instnace_id, VultrInstanceHost(
        ip=instance_ipv4,
        password=password
    ))



class AddVultrInstanceProgram(Program):
    plan = SelectParamType(
        id_map=VULTR_PLANS
    ).param('Plan')

    location = SelectParamType(
        id_map=VULTR_LOCATIONS
    ).param('Location')
    
    def execute(self, context: RequestContext) -> None:
        config = context.config
        api_key = config.get_vultr_api_key()
        assert api_key
        vultr_api = VultrAPI(api_key)
        instance = vultr_api.create_instance(
            self.plan,
            self.location,
            VULTR_DEBIAN_12,
        )
        private_key = PrivateKey.generate()
        public_key = private_key.get_public_key()
        config.add_vultr_instance(VultrInstance(
            id=instance.id,
            public_key=public_key
        ))
        threading.Thread(
            target=load_vultr_instance,
            args=(context.config, vultr_api, instance, private_key)
        ).start()