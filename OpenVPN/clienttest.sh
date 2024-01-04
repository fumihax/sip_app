# Client test Commandline
# 
# Check sipserver in /etc/hosts 
./sip_app -s 172.22.1.41 -u 1000 -t 1001 -p 5200 -d -f ./log/client`date +%m%d%H%M`.txt -m ./openvpn_client.sh
