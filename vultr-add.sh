#!/bin/bash

API_KEY=$(head -n 1 vultr/api-key)
PLAN=$1
LOCATION=$2
OS=2136     # Debian 12

INSTANCE=`curl "https://api.vultr.com/v2/instances" \
  -X POST \
  -H "Authorization: Bearer $API_KEY" \
  -H "Content-Type: application/json" \
  --data "{
    \"region\": \"$LOCATION\",
    \"plan\": \"$PLAN\",
    \"os_id\": $OS,
    \"enable_ipv6\": true,
    \"disable_public_ipv4\": false
  }" | jq -r ".instance"`

INSTANCE_ID=`echo $INSTANCE | jq -r ".id"`
DEFAULT_PASSWORD=`echo $INSTANCE | jq -r ".default_password"`
echo "loading">vultr/instance-$INSTANCE_ID
echo "redirect"

background-setup() {
    trap "" HUP
    # wait until the server is ready
    until [[ `echo $INSTANCE | jq -r ".status"` == 'active' ]]; do
        sleep 10
        INSTANCE=`curl "https://api.vultr.com/v2/instances/$INSTANCE_ID" \
            -X GET \
            -H "Authorization: Bearer $API_KEY" \
        | jq -r ".instance"`
        echo Got $INSTANCE
    done
    
    INSTANCE_IPV4=`echo $INSTANCE | jq -r ".main_ip"`

    # wait until port 22 is open
    until nc -vz $INSTANCE_IPV4 -w 5 22; do
        sleep 5
        echo Port 22 not open! Retrying....
    done

    SERVER_PRIVATE_KEY=`wg genkey`
    SERVER_PUBLIC_KEY=`echo $SERVER_PRIVATE_KEY | wg pubkey`
    PUBLIC_KEY=$(cat const/privatekey | wg pubkey)
    sshpass -p $DEFAULT_PASSWORD scp -o StrictHostKeyChecking=no ./vultr-setup.sh root@$INSTANCE_IPV4:/root/setup.sh
    sshpass -p $DEFAULT_PASSWORD ssh -o StrictHostKeyChecking=no root@$INSTANCE_IPV4 <<EOF
        chmod +x /root/setup.sh
        /root/setup.sh $SERVER_PRIVATE_KEY $PUBLIC_KEY
EOF

    echo "$INSTANCE_ID
$INSTANCE_IPV4
$SERVER_PUBLIC_KEY
$DEFAULT_PASSWORD">vultr/instance-$INSTANCE_ID

}

(background-setup $INSTANCE_ID &) >setup_log 2>setup_errors
