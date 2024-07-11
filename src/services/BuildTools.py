from services.service import Service
from util.bash import bash_get


class BuildTools(Service):
    """
    Maybe we shouldn't install all this
    """
    def __init__(self):
        kernel_version = bash_get("uname -r")
        super().__init__(apt_packages=[
            'build-essential',
            'cmake',
            f'linux-headers-{kernel_version}'
        ])