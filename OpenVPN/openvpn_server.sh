#
# sip_app - OpenVPN(Server) Connector
echo $0 $1 $2 $3 $4 $5

# OLD 
#/usr/local/openvpn-2.0.7/sbin/openvpn --config /usr/local/etc/openvpn-2.0.7/etc/vm2vm.conf --lport $1

#/usr/local/sbin/openvpn --config /usr/local/etc/openvpn/armada_solget.conf --lport $1 --ifconfig $3 $4

#/usr/local/sbin/openvpn --config /usr/local/etc/openvpn/armada_solget.conf --lport $1 --ifconfig 192.168.250.1 192.168.250.2

/usr/local/sbin/openvpn --config ./conf/default.conf --lport $1 --ifconfig $2 $3
