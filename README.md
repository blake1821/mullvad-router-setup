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
    - LAN: Attach to an **Internal Network**
2. Clone this repo into the VM
3. Run `chmod +x setup.sh`
4. Run `sudo ./setup.sh`, and follow the instructions
5. You might also want to edit the grub config for faster bootup time, and configure the VM to launch whenever your host boots up.

## Configuring the Connection to Mullvad Servers
To get the VPN up and running:
1. Visit the website the router exposes to the host via the MAN (host-only) interface. Its URL should just be the IP address of said interface. Log in with the password you created in the setup phase.
2. Log into your Mullvad account, go to Devices, and in the Advanced section, add a new device with the public key shown on the router website.
3. Copy the IPv4 and IPv6 to the router website form, then submit.
![image](https://github.com/blake1821/mullvad-router-setup/assets/124000747/17ad1bdd-6018-4c63-9736-278049cba0da)



## Use the SOCKS5 proxy
I should make it clear that this utility isn't intended to route all your host's traffic through the VPN tunnel. To browse the web privately on the host, you need to configure your browser to use the proxy server. Connect using the MAN IP and port `1080`.
![image](https://github.com/blake1821/mullvad-router-setup/assets/124000747/f91dcfd2-9e6b-4fd4-8b0d-dc9ea445a0e1)

## Configuring Downstream Hosts
Simply attach downstream VMs to the same internal network.
![image](https://github.com/blake1821/mullvad-router-setup/assets/124000747/e7acedad-0ee3-4abc-af43-52544aecada8)

## Configuring Port Forwarding
On the router website, you can configure port forwarding from your host to one of the VMs in the internal network.
![image](https://github.com/blake1821/mullvad-router-setup/assets/124000747/67c5e43f-6c54-4b69-9cd0-dfc2ad583321)


```bash
# This is executed on the host machine
# 192.168.56.103 is the router's IP
# 1022 is a port forwarded from the router to a VM
ssh user@192.168.56.103 -p 1022
```
