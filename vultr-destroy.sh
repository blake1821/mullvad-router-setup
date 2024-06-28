#!/bin/bash

API_KEY=$(head -n 1 vultr/api-key)
INSTANCE_ID=$1

curl "https://api.vultr.com/v2/instances/$INSTANCE_ID" \
  -X DELETE \
  -H "Authorization: Bearer $API_KEY"
rm vultr/instance-$INSTANCE_ID
echo "redirect"
