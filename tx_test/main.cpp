
#include "hackrf.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>

#define PI 3.14159265

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
    std::cout << "Stopping Tx" << std::endl;
    if ( ! check_error( hackrf_stop_tx( device ) ) )
        std::cout << "Error: failed to stop tx mode!" << std::endl;
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
// Callback function for tx samples
//
//262144
double t = 0.0;
double dt = 1/1000000.0;
bool print_signal = true;
double df = 10000;
int sample_block_cb_fn(hackrf_transfer* transfer)
{
    for (int ii = 0; ii < transfer->valid_length; ii+=2)
    {
        double i = 128.0 * cos( 2.0 * PI * df * t );  // I
        double q = 128.0 * sin( 2.0 * PI * df * t );  // Q
        
        uint8_t i8 = uint8_t(i);
        uint8_t q8 = uint8_t(q);
        
        transfer->buffer[ii+0] = i8;
        transfer->buffer[ii+1] = q8;
        
        t = t+dt;
        if ( t >= 1.0 )
        {
            t = 0.0;
            df = -df;
        }
    }
    
    return 0;
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
    if ( ! check_error( hackrf_set_freq( device, 905000000 ) ) )
        return EXIT_FAILURE;

    std::cout << "Setting sample rate..." << std::endl;
    if ( ! check_error( hackrf_set_sample_rate( device, 1000000 ) ) )
        return EXIT_FAILURE;

    std::cout << "Setting gain..." << std::endl;
    if ( ! check_error( hackrf_set_lna_gain( device, 0 ) ) )
        return EXIT_FAILURE;

    std::cout << "Disabling amp..." << std::endl;
    if ( ! check_error( hackrf_set_amp_enable( device, 0 ) ) )
        return EXIT_FAILURE;

    std::cout << "Disabling txvga..." << std::endl;
    if ( ! check_error( hackrf_set_txvga_gain( device, 0 ) ) )
        return EXIT_FAILURE;


    // Setup the interrupt signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    // Start receiving data
    std::cout << "Starting Tx" << std::endl;
    if ( ! check_error( hackrf_start_tx( device, sample_block_cb_fn, (void*)(NULL) ) ) )
        return EXIT_FAILURE;

    //  Wait a while
    sleep(30);

    // Release the device
    cleanup();

    return 0;
}





