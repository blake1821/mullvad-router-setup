#!/bin/bash

# Web page script

endpoint_ip=$(wg | grep 'endpoint:' | awk '{ print $2 }' | cut -d ':' -f 1) 2>&1
dhcp_mac_ip_hostname_list=$(dhcp-lease-list --parsable 2>/dev/null | awk '{print $2, $4, $6}')

cat <<EOF
<html>
    <head>
        <title>Mullvad Router</title>
    </head>
<body>
    <h1>Mullvad VPN Router</h1>
    <p>Connected to $endpoint_ip</p>
    <form method="GET" action="/restart">
        <input type="submit" value="restart">
    </form>
    <hr/>
    <h2>Client List</h2>
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
    <h2>Port Forwards</h2>
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

    <h3>Forward a new port:</h3>
    <form method="GET" action="/add-port-fw">
        <label for="sp">Source Port: </label><input required type="number" name="sp" id="sp"/><br/>
        <label for="da">Destination Address: </label><input required type="text" name="da" id="da"/><br/>
        <label for="dp">Destination Port: </label><input required type="number" name="dp" id="dp"/><br/>
        <input type="submit" value="add">
    </form>

    <br/>
    <br/>
    <br/>

    <em>Note: This "website" is extremely insecure, but it should only be accessible via the MAN interface.</em>
</body>
</html>
EOF