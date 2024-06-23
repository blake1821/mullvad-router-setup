# mullvad-router-setup
These scripts are intended to configure a virtual machine to act as a basic router and wireguard client for your dekstop. The configured VM will:
- Expose a SOCKS5 proxy to the host
- Serve as a gateway router for downstream hosts on the internal network
- Optionally forward specified ports to downstream hosts

Upstream traffic for the proxy and gateway will be routed through the VPN tunnel.

With this setup, you can:
- Configure your web browser for private and secure browsing using the SOCKS proxy
- Keep VMs completely isolated from your home network
- Connect to VMs via the router through explicitly forwarded ports

![image](https://github.com/blake1821/mullvad-router-setup/assets/124000747/6f87e400-1a27-4cb7-8519-eaf6eedcfc19)

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

## Configuring Port Forwarding
To configure port forwarding from your host to one of the VMs in the internal network, visit the website the router exposes to the host via the MAN (host-only) interface. Its url should just be the IP address of said interface:
![image](https://github.com/blake1821/mullvad-router-setup/assets/124000747/620fd111-4553-4ae0-b958-41ed14e36404)

From here, you can forward a port from the MAN interface to any downstream client.

```bash
# This is executed on the host machine
# 192.168.56.103 is the router's IP
# 1022 is a port forwarded from the router to a VM
ssh user@192.168.56.103 -p 1022
```
