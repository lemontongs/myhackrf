#! /bin/bash

if [[ $# -lt 1 ]]
then
    echo "Usage: $0 <duration>"
    exit 1
fi


#
# Make the programs
#
if [[ ! $(make) ]]
then
    echo "Build error, aborting."
    exit 1
fi

DURATION=$1
SAMPLE_RATE=20000000
NUM_SAMPLES_RX=$(echo "(${SAMPLE_RATE} * ${DURATION})/1" | bc)
DATA_FILE=rx.dat


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
if [[ -e ${DATA_FILE} ]]
then
    rm ${DATA_FILE}
fi

hackrf_transfer -r ${DATA_FILE} -d 22be1 -f 2480000000 -a 1 -l 40 -g 0 -s ${SAMPLE_RATE} -n ${NUM_SAMPLES_RX}
#hackrf_transfer -r ${DATA_FILE} -d 22be1 -f 916000000 -a 1 -l 40 -g 0 -s ${SAMPLE_RATE} -n ${NUM_SAMPLES_RX}

# Kill the transmitter
kill -INT $TX_PID


if [ ! -e ${DATA_FILE} ] || [ $(du -b ${DATA_FILE} | awk '{print $1}') -lt 1 ]
then
    echo "Failed to receive data"
    exit 1
fi

#
# Process the received file
#
matlab -nodesktop -nosplash -r plot_as_rdm




