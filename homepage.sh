#!/bin/bash

# Web page script

shopt -s nullglob

endpoint_ip=$(wg | grep 'endpoint:' | awk '{ print $2 }' | cut -d ':' -f 1) 2>&1
if [[ -z $endpoint_ip ]]; then
    connected_message="&#10060; Not connected"
else
    connected_message="&#9989; Connected to <span class=\"code\">$endpoint_ip</span>"
fi
dhcp_mac_ip_hostname_list=$(dhcp-lease-list --parsable 2>/dev/null | awk '{print $2, $4, $6}')
public_key=$(cat const/privatekey | wg pubkey)

cat <<EOF
<html>
<head>
    <title>Mullvad Router</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <h1>Mullvad VPN Router</h1>
    <div class="centered">
    <div>
        <p style="margin: 0px;">$connected_message</p>
        <form method="POST" action="/restart">
            <input class="margin-top" type="submit" value="restart">
        </form>
    </div>
    </div>
    <hr/>
    <h3>Mullvad Devices</h3>
    <div class="centered">
    <table>
EOF
        mkdir mullvad 2>/dev/null
        cd mullvad
        for mv_id in *; do
            echo "<tr>"
            exec 3<$mv_id
            location=$(head -n 1 <&3)
            cat <<EOF
            <td>$location</td>
            <td>
                <form method="POST" action="/configure">
                    <input type="hidden" name="type" value="mullvad">
                    <input type="hidden" name="id" value="$mv_id">
                    <input type="submit" value="set peer">
                </form>
            </td>
            <td>
                <form method="POST" action="/mullvad-forget">
                    <input type="hidden" name="id" value="$mv_id">
                    <input type="submit" value="forget">
                </form>
            </td>
EOF
            exec 3<&-
            echo "</tr>"
        done
        cd ..
cat <<EOF
    </table>
    </div>
    <h3>Add a Mullvad Device</h3>
    <p>The public key is <span class="code">$public_key</code></p>
    <form method="POST" action="/mullvad-add" class="centered">
    <div>
        <table>
        <tr>
            <td>IPv4:</td>
            <td><input name="my4" type="text" placeholder="23.7.15.131/32"></td>
        </tr>
        <tr>
            <td>IPv6:</td>
            <td><input name="my6" type="text" placeholder="fc00:bcbc::1/128"></td>
        </tr>
        <tr>
            <td>Location: </td>
            <td><input name="location" type="text" placeholder="us"></td>
        </tr>
        </table>
        <input class="margin-top" type="submit" value="add">
    </div>
    </form>
    <hr/>
EOF
    if [[ -f vultr/api-key ]]; then
cat <<EOF
    <h3>Vultr Instances</h3>
    <div class="centered">
    <table>
EOF
        cd vultr
        for instance_file in instance-*; do
            echo "<tr>"
            exec 3<$instance_file
            first_line=$(head -n 1 <&3)
            if [[ $first_line == 'loading' ]]; then
                echo "<td>Loading...</td> <td></td> <td></td> <td></td>"
            else
                instance_id=$first_line
                instance_ipv4=$(head -n 1 <&3)
                instance_pubkey=$(head -n 1 <&3)
                cat <<EOF
                <td>$instance_ipv4</td>
                <td>
                    <form method="POST" action="/vultr-details">
                        <input type="hidden" name="instance-id" value="$instance_id">
                        <input type="submit" value="details">
                    </form>
                </td>
                <td>
                    <form method="POST" action="/configure">
                        <input type="hidden" name="type" value="vultr">
                        <input type="hidden" name="id" value="$instance_id">
                        <input type="submit" value="set peer">
                    </form>
                </td>
                <td>
                    <form method="POST" action="/vultr-destroy">
                        <input type="hidden" name="instance-id" value="$instance_id">
                        <input type="submit" value="destroy">
                    </form>
                </td>
EOF
            fi
            exec 3<&-
            echo "</tr>"
        done
        cd ..
cat <<EOF
    </table>
    </div>
    <h3>Create New Instance</h3>
    <em style="font-size:8pt">Use ./vultr-list.sh and then modify ./homepage.sh to add more options</em><br/>
    <form class="centered" method="POST" action="/vultr-add">
    <div>
        <table>
            <tr>
                <td>Plan: </td>
                <td><select required name="plan">
                    <option value="vc2-1c-1gb">VC2, 1c, 1GB, 1TB Bandwidth, \$5/month</option>
                    <option value="vhp-1c-1gb-amd">VHP-AMD, 1c, 1GB, 2TB Bandwidth, \$6/month</option>
                </select></td>
            </tr>
            <tr>
                <td>Location: </td>
                <td><select required name="location">
                    <option value="ewr">New Jersey</option>
                    <option value="ord">Chicago</option>
                    <option value="dfw">Dallas</option>
                    <option value="sea">Seattle</option>
                    <option value="lax">Los Angeles</option>
                    <option value="atl">Atlanta</option>
                    <option value="sjc">Silicon Valley</option>
                    <option value="yto">Toronto</option>
                    <option value="mia">Miami</option>
                </select></td>
            </tr>
        </table>
        <input class="margin-top" type="submit" value="add">
    </div>
    </form>

EOF
    else
cat <<EOF
    <div class="centered">
    <div>
        <p>Link your Vultr account</p>
        <form method="POST" action="/vultr-link">
            <table>
                <tr>
                    <td>API Key: </td>
                    <td><input required type="text" name="api-key"/></td>
                </tr>
            </table>
            <input class="margin-top" type="submit" value="link">
        </form>
    </div>
    </div>
EOF
    fi
cat <<EOF
    <hr/>
    <h2>Client List</h2>
    <div class="centered">
    <table>
        <tr>
            <th>MAC</th>
            <th>IP</th>
            <th>Hostname</th>
        </tr>
EOF
        while read -r line; do
                echo "<tr>
                    <td>$(echo $line | cut -d ' ' -f 1)</td>
                    <td>$(echo $line | cut -d ' ' -f 2)</td>
                    <td>$(echo $line | cut -d ' ' -f 3)</td>
                </tr>"
        done <<<$dhcp_mac_ip_hostname_list
cat <<EOF
    </table>
    </div>
    <h2>Port Forwards</h2>
    <div class="centered">
    <table>
        <tr>
            <th>Source Port</th>
            <th></th>
            <th>Destination Address</th>
            <th>Destination Port</th>
            <th></th>
        </tr>
EOF
    mkdir port-fw 2>/dev/null
    cd port-fw
    for src_port in *
    do
            echo "<tr>
            <td>$src_port</td>
            <td>&#8594;</td>
            <td>$(head $src_port -n 1)</td>
            <td>$(tail $src_port -n 1)</td>
            <td>
                    <form method=\"POST\" action=\"/add-port-fw\">
                    <input type=\"hidden\" name=\"sp\" value=\"$src_port\"/>
                    <input type=\"hidden\" name=\"da\" value=\"delete\"/>
                    <input type=\"hidden\" name=\"dp\" value=\"delete\"/>
                    <input type=\"submit\" value=\"delete\">
                    </form>
            </td>
            </tr>"
    done
    cd ..

cat <<EOF
    </table>
    </div>

    <h3>Forward a new port:</h3>
    <div class="centered">
    <form method="POST" action="/add-port-fw">
        <table>
            <tr>
                <td><label for="sp">Source Port: </label></td>
                <td><input required type="number" name="sp" id="sp"/></td>
            </tr>
            <tr>
                <td><label for="da">Destination Address: </label></td>
                <td><input required type="text" name="da" id="da"/></td>
            </tr>
            <tr>
                <td><label for="dp">Destination Port: </label></td>
                <td><input required type="number" name="dp" id="dp"/></td>
            </tr>
        </table>
        <input class="margin-top" type="submit" value="add">
    </form>
    </div>
    <br/>
    <br/>
    <br/>

    <em>Note: This "website" is extremely insecure, but it should only be accessible via the MAN interface.</em>
</body>
</html>
EOF