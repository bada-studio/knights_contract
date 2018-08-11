if [ "$1" = "real" ]
then
  url=https://rpc.eosys.io:443
elif [ "$1" = "beta" ]
then
  url=http://121.168.149.101:8888
elif [ "$1" = "local" ]
then
  url=http://127.0.0.1:8888
else
  echo "need phase"
  exit 0
fi

cleos -u $url set contract eosknightsio knights -p eosknightsio
