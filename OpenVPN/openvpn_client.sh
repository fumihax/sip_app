#!/bin/sh
#
# sip_app - OpenVPN(Client) Connector
#echo $0 $1 $2 $3 $4 $5

# OLD 
#/usr/local/openvpn-2.0.7/sbin/openvpn --config /usr/local/etc/openvpn-2.0.7/etc/vm2vm.conf --remote $1 --rport $2

while [ -d . ]; do
sleep 1
done
/usr/local/sbin/openvpn --config ./conf/default.conf --remote $1 --rport $2 --ifconfig $3 $4
