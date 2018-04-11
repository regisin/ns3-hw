echo "No simulation to run at start-up configured"
echo "Edit me at: $(dirname $(readlink -f $0))/`basename $0`"

# Uncomment/edit the line below to run the desired script at startup:
./waf --run="test-ina --time=10 --charge=8000" > test-ina.log 2>&1
