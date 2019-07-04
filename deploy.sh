if [ "$1" = "real" ]
then
  url=https://rpc.eosys.io:443
  contract=eosknightsio
elif [ "$1" = "beta" ]
then
  url=https://api-kylin.eoslaomao.com
  contract=eosknightsio
elif [ "$1" = "local" ]
then
  url=http://127.0.0.1:8888
  contract=eosknightsio
else
  echo "need phase"
  exit 1
fi

cleos -u $url set contract $contract knights -p $contract
