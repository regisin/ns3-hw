counter=0
PROCS=1

for run in {1..100}
do
    echo "run $run"
    ./waf --run="consumption --id=$run" >> $run.csv 2>&1 &
    counter=$counter+1
    if ! (( counter % PROCS ))
    then
        wait
    fi
done