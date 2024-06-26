#!/bin/bash

# Web page script

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
        <form method="GET" action="/restart">
            <input class="margin-top" type="submit" value="restart">
        </form>
    </div>
    </div>
    <hr/>
    <h2>Configure the Peer Connection</h2>
    <p>The public key is <span class="code">$public_key</code></p>
    <form method="GET" action="/configure" class="centered">
        <div>
        <table>
        <tr>
            <td>Peer type:</td>
            <td>
                <select id="endpoint-type" name="endpoint-type" onchange="change()">
                    <option value="mullvad">Mullvad</option>
                    <option value="custom">Custom</option>
                </select>
            </td>
        </tr>
        <tr>
            <td>IPv4:</td>
            <td><input name="my4" type="text" placeholder="23.7.15.131/32"></td>
        </tr>
        <tr>
            <td>IPv6:</td>
            <td><input name="my6" type="text" placeholder="fc00:bcbc::1/128"></td>
        </tr>
        <tbody id="ep:mullvad" style="">
            <tr>
                <td>Location: </td>
                <td><input name="location" type="text" placeholder="us"></td>
            </tr>
        </tbody>
        <tbody id="ep:custom" style="display: none;">
            <tr>
                <td>Peer IP: </td>
                <td><input name="endpoint-ip" type="text"></td>
            </tr>
            <tr>
                <td>Peer Pubkey: </td>
                <td><input name="endpoint-pubkey" type="text"></td>
            </tr>
        </tbody>
        </table>
        <input class="margin-top" type="submit" value="submit">
        <script>
            function change(){
                if(document.getElementById('endpoint-type').value == 'mullvad'){
                    document.getElementById('ep:mullvad').style = '';
                    document.getElementById('ep:custom').style = 'display: none;';
                }else{
                    document.getElementById('ep:custom').style = '';
                    document.getElementById('ep:mullvad').style = 'display: none;';
                }
            }
        </script>
        </div>
    </form>
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
    shopt -s nullglob
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
                    <form method=\"GET\" action=\"/add-port-fw\">
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
    <form method="GET" action="/add-port-fw">
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
        <input type="submit" value="add">
    </form>
    </div>
    <br/>
    <br/>
    <br/>

    <em>Note: This "website" is extremely insecure, but it should only be accessible via the MAN interface.</em>
</body>
</html>
EOF