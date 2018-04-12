#echo "No simulation to run at start-up configured"
#echo "Edit me at: $(dirname $(readlink -f $0))/`basename $0`"

# Edit the line below to run the desired script at startup:
./waf --run="test-split-access_relay" > test-ina.log 2>&1
