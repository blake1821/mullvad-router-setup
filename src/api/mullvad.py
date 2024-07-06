import requests

from data.mullvad import MullvadLocation, MullvadRelay

def get_mullvad_relays(location: MullvadLocation):
    response = requests.get("https://api.mullvad.net/app/v1/relays").json()
    return [
        MullvadRelay(
            ip=relay['ipv4_addr_in'],
            pubkey=relay['public_key']
        )
        for relay in response['wireguard']['relays']
        if relay['location'].startswith(location.get_prefix())
    ]