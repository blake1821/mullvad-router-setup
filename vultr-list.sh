#!/bin/bash

# Command Line utitlity to fetch the locations and plans and put them in the vultr folder

API_KEY=$(head -n 1 vultr/api-key)

curl "https://api.vultr.com/v2/plans?type=vhp" \
  -X GET \
  -H "Authorization: Bearer ${API_KEY}" >vultr/plans-vhp

curl "https://api.vultr.com/v2/plans?type=vc2" \
  -X GET \
  -H "Authorization: Bearer ${API_KEY}" >vultr/plans-vc2

curl "https://api.vultr.com/v2/regions" \
  -X GET \
  -H "Authorization: Bearer ${API_KEY}" >vultr/regions

curl "https://api.vultr.com/v2/os" \
  -X GET \
  -H "Authorization: Bearer ${API_KEY}" >vultr/os


echo Look in ./vultr/ for the downloaded files