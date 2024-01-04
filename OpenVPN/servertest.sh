# Server test Commandline
# Check sipserver in /etc/hosts

./sip_app -s 202.26.159.130 -p 5100 -u 2500 -v 8139 -l 192.168.250.1 -r 192.168.250.2 -d -f ./log/server`date +%m%d%H%M`.txt -m ./openvpn_server.sh
