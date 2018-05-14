#!/bin/bash
nodenumber=$(tail -c 3 /etc/hostname)

sleep 7 # on avg. this is enough to have connection, otherwise will not solve github.com address
# update source-code
git fetch --all
git reset --hard origin/master

# replace placeholders with correct node number for ip, and correct command to execute in run.sh
sed -i -e "s/XX/$nodenumber/g" ./scratch/client.cc
sed -i -e "s/XX/$nodenumber/g" ./scratch/relay.cc
cmd=$(cat config.txt | grep $nodenumber: | cut -d ':' -f2)
echo $cmd
cmd=${cmd//\\/\\\\}
echo $cmd
cmd=${cmd//\//\\/}
echo $cmd
cmd=${cmd//./\\.}
echo $cmd
cmd=${cmd//&/\\&}
echo $cmd
sed -i -e "s/#command#/$cmd/g" ./run.sh
