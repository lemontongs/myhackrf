#! /bin/bash


#
# Make the programs
#
if [[ ! $(make) ]]
then
    echo "Build error, aborting."
    exit 1
fi

DURATION=0.5
SAMPLE_RATE=20000000
NUM_SAMPLES_RX=$(echo "(${SAMPLE_RATE} * ${DURATION})/1" | bc)
DATA_FILE=rx.dat
DATA_MUTEX_FILE=rx.dat.lock



#
# Start the file processor
#
matlab -nodesktop -nosplash -r plot_and_wait &
MATLAB_PID=$!

# Allow it to check for hardware and begin
sleep 5

if ! kill -0 $MATLAB_PID
then
    echo "Failed to start matlab!"
    exit 1
fi


#
# Start the transmitter
#
./radar_tx &
TX_PID=$!

# Allow it to check for hardware and begin
sleep 1

if ! kill -0 $TX_PID
then
    echo "Failed to start transmitter!"
    exit 1
fi


#
# Start the receiver
#

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT

function ctrl_c()
{
    echo "Stopping"
    kill -INT $TX_PID
    kill -INT $MATLAB_PID
    exit 0
}


while kill -0 $MATLAB_PID
do
    # If the file exists wait for matlab to process it, otherwise, create a new one
    if [[ -e ${DATA_MUTEX_FILE} ]]
    then
        sleep 1
    else
        hackrf_transfer -r ${DATA_FILE} -d 22be1 -f 2480000000 -a 1 -l 40 -g 0 -s ${SAMPLE_RATE} -n ${NUM_SAMPLES_RX}
        touch $DATA_MUTEX_FILE
    fi
done

kill -INT $TX_PID
rm $DATA_FILE
rm $DATA_MUTEX_FILE



