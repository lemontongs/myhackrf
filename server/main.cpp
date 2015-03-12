
#include "hackrf.h"
#include "zhelpers.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>

hackrf_device* device = NULL;

//
// Check for hackrf error message
//
bool check_error(int error)
{
    hackrf_error hrf_e = hackrf_error(error);

    if ( HACKRF_SUCCESS == hrf_e )
        return true;
    else
        std::cout << "Error: " << hackrf_error_name( hrf_e ) << std::endl;
    return false;
}

void cleanup()
{
    std::cout << "Stopping Rx" << std::endl;
    if ( ! check_error( hackrf_stop_rx( device ) ) )
        std::cout << "Error: failed to stopping rx mode!" << std::endl;
    if ( ! check_error( hackrf_close(device) ) )
        std::cout << "Error: failed to release device!" << std::endl;

    hackrf_exit();
    std::cout << "Done" << std::endl;
    exit(1);
}

void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    cleanup();
}


//
// Callback function for rx samples
//
int sample_block_cb_fn(hackrf_transfer* transfer)
{
    // send the packet out over the network
    zmq::socket_t * publisher = (zmq::socket_t *)transfer->rx_ctx;
    zmq::message_t message((void*)transfer->buffer, transfer->valid_length, NULL);
    publisher->send(message);

    return 0;
}

//
// Handle received message. Return false to exit main loop.
//
bool process_message( std::string & message )
{
    std::cout << "Received: '" << message << "'" << std::endl;

    std::vector<std::string> fields;
    std::string temp;
    std::stringstream s( message );
    while( s >> temp )
        fields.push_back( temp );

    if ( fields.size() < 1 )
        return true;

    if ( fields[0] == "quit" )
        return false;
    if ( fields[0] == "tune" )
    {
        if ( fields.size() >= 2 )
            std::cout << "tuning to: " << fields[1] << std::endl;
    }
}

//
// MAIN
//
int main()
{
    uint8_t board_id = BOARD_ID_INVALID;
    char version[255 + 1];
    read_partid_serialno_t read_partid_serialno;

    if ( ! check_error( hackrf_init() ) )
        return EXIT_FAILURE;

    if ( ! check_error( hackrf_open( &device ) ) )
        return EXIT_FAILURE;

    printf("Found HackRF board.\n");

    if ( ! check_error( hackrf_board_id_read(device, &board_id) ) )
        return EXIT_FAILURE;

    if ( ! check_error( hackrf_version_string_read(device, &version[0], 255) ) )
        return EXIT_FAILURE;

    printf("Firmware Version: %s\n", version);

    if ( ! check_error( hackrf_board_partid_serialno_read(device, &read_partid_serialno) ) )
        return EXIT_FAILURE;

    printf("Part ID Number: 0x%08x 0x%08x\n",
            read_partid_serialno.part_id[0],
            read_partid_serialno.part_id[1]);
    printf("Serial Number: 0x%08x 0x%08x 0x%08x 0x%08x\n",
            read_partid_serialno.serial_no[0],
            read_partid_serialno.serial_no[1],
            read_partid_serialno.serial_no[2],
            read_partid_serialno.serial_no[3]);

    std::cout << "Tuning..." << std::endl;
    if ( ! check_error( hackrf_set_freq( device, 433900000 ) ) )
        return EXIT_FAILURE;

    std::cout << "Setting sample rate..." << std::endl;
    if ( ! check_error( hackrf_set_sample_rate( device, 1000000 ) ) )
        return EXIT_FAILURE;

    std::cout << "Setting gain..." << std::endl;
    if ( ! check_error( hackrf_set_lna_gain( device, 40 ) ) )
        return EXIT_FAILURE;

    // Setup the interrupt signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    // Start the server bits
    zmq::context_t pub_context(1);
    zmq::socket_t publisher(pub_context, ZMQ_PUB);
    publisher.bind("tcp://*:5555");

    zmq::context_t recv_context(1);
    zmq::socket_t receiver(recv_context, ZMQ_REP);
    receiver.bind("tcp://*:5556");

    // Start receiving data
    std::cout << "Starting Rx" << std::endl;
    if ( ! check_error( hackrf_start_rx( device, sample_block_cb_fn, (void*)(&publisher) ) ) )
        return EXIT_FAILURE;

    //  Wait for next request from client
    std::string msg = s_recv(receiver);
    while ( process_message( msg ) )
        (void)0;

    // Release the device
    cleanup();

    return 0;
}





