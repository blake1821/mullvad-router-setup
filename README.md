# mullvad-router-setup
These scripts are intended to configure a virtual machine to act as a basic router and wireguard client for your dekstop. The configured VM will:
- Expose a SOCKS5 proxy to the host
- Serve as a gateway router for downstream hosts on the internal network

Upstream traffic for the proxy and gateway will be routed through the VPN tunnel.

With this setup, you can:
- Configure your web browser for private and secure browsing using the SOCKS proxy
- Keep VMs completely isolated from your home network

## Installation
1. Create a fresh Debian 12 virtual machine with 3 network interfaces:
    - WAN: Attach to **NAT**
    - MAN: Attach to **Host-only adapter**
        - used for remote configuration and SOCKS5 proxy
    - LAN: Attach to an **Internal Network**
2. Clone this repo into the VM
3. Run `chmod +x setup.sh`
4. Run `sudo ./setup.sh`
5. You might also want to edit the grub config for faster bootup time, and configure the VM to launch whenever your host boots up.


