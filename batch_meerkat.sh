
clients=(
        "10.1.0.8"
        "10.1.0.9"
        "10.1.0.29" "10.1.0.30" "10.1.0.31" "10.1.0.44" # client16 ~ client19
        "10.1.0.32" "10.1.0.33" "10.1.0.34" "10.1.0.35" # client20 ~ client23
        "10.1.0.36" "10.1.0.37" "10.1.0.38" "10.1.0.39" # client24 ~ client27
        "10.1.0.40" "10.1.0.41" "10.1.0.42" "10.1.0.43" # client28 ~ client31
)

cmd1="cd ~ && rm -rf meerkat && scp -r azureuser@10.1.0.7:/home/azureuser/meerkat ."
cmd2="sudo skill meerkat_server;sudo pkill retwisClient;sudo pkill meerkat_server;sudo skill retwisClient; sudo skill benchClient; sudo pkill benchClient; sleep 1; "
cmd3="sudo rm -rf ~/meerkat/logs/* && sudo rm -rf /tmp/*"
cmd4="" #"cd ~/meerkat && sudo ./install.sh"
cmd5="bash ~/meerkat/init.sh"
cmd6=""

# for local machine 10.1.0.7
if [ $1 == 'scp' ]; then
	:
elif [ $1 == 'kill' ]; then
    echo "kill local"
    eval	$cmd2
elif [ $1 == 'del' ]; then
    echo "del local"
    eval	$cmd3
elif [ $1 == 'install' ]; then 
	:
elif [ $1 == 'init' ]; then
    echo "init local"
    eval	$cmd5
else
        :
fi

for host in ${clients[@]}
do
  if [ $1 == 'scp' ]; then
    echo "scp to $host"
    ssh $host "$cmd1" &
  elif [ $1 == 'kill' ]; then
    echo "kill host $host"
    ssh $host "$cmd2" &
  elif [ $1 == 'del' ]; then
      echo "delete logs host $host"
      ssh $host "$cmd3" &
  elif [ $1 == 'install' ]; then 
    echo "install meerkat"
    ssh $host "$cmd4" &
  elif [ $1 == 'init' ]; then
    echo "init: $host"
    ssh $host "$cmd5" &
  else
	  :
  fi
done

echo "Wait for jobs..."
# wait for completion
FAIL=0

for job in `jobs -p`
do
    wait $job || let "FAIL+=1"
done

if [ "$FAIL" == "0" ];
then
    echo "YAY!"
else
    echo "FAIL! ($FAIL)"
fi


