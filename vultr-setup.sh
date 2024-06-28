# This is executed on the Vultr server

PRIVATE_KEY=$1
PEER_PUBKEY=$2

apt update
apt upgrade
apt -y install wireguard
ufw allow 51820
echo "net.ipv4.ip_forward=1" >>/etc/sysctl.conf
echo "net.ipv6.conf.all.forwarding=1" >>/etc/sysctl.conf
sysctl -p

WAN_IFNAME=`ip -o -4 route show to default | awk '{print $5}'`

cat >/etc/wireguard/wg0.conf <<EOF
[Interface]
Address = 10.0.33.1/24, fc00:3333::1/120
ListenPort = 51820
PrivateKey = $PRIVATE_KEY

PostUp = iptables -A FORWARD -i wg0 -j ACCEPT; iptables -t nat -A POSTROUTING -o $WAN_IFNAME -j MASQUERADE; ip6tables -A FORWARD -i wg0 -j ACCEPT; ip6tables -t nat -A POSTROUTING -o $WAN_IFNAME -j MASQUERADE
PostDown = iptables -D FORWARD -i wg0 -j ACCEPT; iptables -t nat -D POSTROUTING -o $WAN_IFNAME -j MASQUERADE; ip6tables -D FORWARD -i wg0 -j ACCEPT; ip6tables -t nat -D POSTROUTING -o $WAN_IFNAME -j MASQUERADE

[Peer]
PublicKey = $PEER_PUBKEY
AllowedIPs = 10.0.33.2/24, fc00:3333::2/120
EOF

sudo systemctl enable wg-quick@wg0
sudo systemctl start wg-quick@wg0