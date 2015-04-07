
#include <iostream>

#include "HackRFDevice.h"

HackRFDevice::HackRFDevice()
{
    m_is_initialized = false;
    m_device_mode    = STANDBY_MODE;    // mode: 0=standby 1=Rx 2=Tx
    m_fc_hz          = 462610000;       // center freq
    m_fs_hz          = 8000000;         // sample rate
    m_lna_gain       = 0;
    m_amp_enable     = 0;
    m_antenna_enable = 0;
    m_rxvga_gain     = 0;
    m_txvga_gain     = 0;
}

HackRFDevice::~HackRFDevice()
{
    cleanup();
}

bool HackRFDevice::initialize()
{
    uint8_t board_id = BOARD_ID_INVALID;
    char version[255 + 1];
    read_partid_serialno_t read_partid_serialno;

    if ( ! check_error( hackrf_init() ) )
        return false;

    if ( ! check_error( hackrf_open( &m_device ) ) )
    {
        hackrf_exit();
        return false;
    }

    std::cout << "Found HackRF board" << std::endl;

    if ( ! check_error( hackrf_board_id_read( m_device, &board_id ) ) )
        std::cout << "ERROR: reading board ID!" << std::endl;

    if ( ! check_error( hackrf_version_string_read( m_device, &version[0], 255) ) )
        std::cout << "ERROR: reading board version!" << std::endl;
    else
        std::cout << "Firmware Version: " << version << std::endl;

/*    if ( ! check_error( hackrf_board_partid_serialno_read( m_device, &read_partid_serialno ) ) )
        return false;

    printf("Part ID Number: 0x%08x 0x%08x\n",
            read_partid_serialno.part_id[0],
            read_partid_serialno.part_id[1]);
    printf("Serial Number: 0x%08x 0x%08x 0x%08x 0x%08x\n",
            read_partid_serialno.serial_no[0],
            read_partid_serialno.serial_no[1],
            read_partid_serialno.serial_no[2],
            read_partid_serialno.serial_no[3]);
*/

    m_is_initialized = true;
    
    //
    // Set device parameters
    //
    if ( ! tune( m_fc_hz ) ||
         ! set_sample_rate( m_fs_hz ) ||
         ! set_lna_gain( m_lna_gain ) ||
         ! set_amp_enable( m_amp_enable ) ||
         ! set_txvga_gain( m_txvga_gain ) )
    {
        cleanup();
        return false;
    }
        
    m_is_initialized = true;
    return true;
}

bool HackRFDevice::start_Rx( hackrf_sample_block_cb_fn callback, void* args = NULL )
{
    if ( m_is_initialized && m_device_mode == STANDBY_MODE )
    {
        std::cout << "Starting Rx" << std::endl;
        if ( check_error( hackrf_start_rx( m_device, callback, args ) ) )
        {
            m_device_mode == RX_MODE;
            return true;
        }
    }
    return false;
}

bool HackRFDevice::stop_Rx()
{
    if ( m_is_initialized && m_device_mode == RX_MODE )
    {
        std::cout << "Stopping Rx" << std::endl;
        if ( ! check_error( hackrf_stop_rx( m_device ) ) )
        {
            std::cout << "Error: failed to stop rx mode!" << std::endl;
            return false;
        }
        m_device_mode == STANDBY_MODE;
        return true;
    }
    return false;
}

bool HackRFDevice::start_Tx( hackrf_sample_block_cb_fn callback, void* args = NULL )
{
    if ( m_is_initialized && m_device_mode == STANDBY_MODE )
    {
        std::cout << "Starting Tx" << std::endl;
        if ( check_error( hackrf_start_tx( m_device, callback, args ) ) )
        {
            m_device_mode == TX_MODE;
            return true;
        }
    }
    return false;
}

bool HackRFDevice::stop_Tx()
{
    if ( m_is_initialized && m_device_mode == TX_MODE )
    {
        std::cout << "Stopping Tx" << std::endl;
        if ( ! check_error( hackrf_stop_tx( m_device ) ) )
        {
            std::cout << "Error: failed to stop tx mode!" << std::endl;
            return false;
        }
        m_device_mode == STANDBY_MODE;
        return true;
    }
    return false;
}

bool HackRFDevice::tune( uint64_t fc_hz )
{
    if ( m_is_initialized )
    {
        if ( fc_hz >= 20000000 && fc_hz <= 6000000000 )
        {
            std::cout << "Tuning to " << fc_hz << std::endl;
            if ( check_error( hackrf_set_freq( m_device, fc_hz ) ) )
            {
                m_fc_hz = fc_hz;
                return true;
            }
        }
        else
        {
            std::cout << "WARNING: invalid tune frequency: " << fc_hz << std::endl;
        }
    }
    return false;
}

bool HackRFDevice::set_sample_rate( double fs_hz )
{
    if ( m_is_initialized )
    {
        switch ( int( fs_hz ) )
        {
        case 20000000 :
        case 16000000 :
        case 12500000 :
        case 10000000 :
        case  8000000 :
            return force_sample_rate( fs_hz );
        default:
            std::cout << "WARNING: invalid sample rate: " << fs_hz << std::endl;
        }
    }
    return false;
}

bool HackRFDevice::force_sample_rate( double fs_hz )
{
    if ( m_is_initialized )
    {
        std::cout << "Setting sample rate to " << fs_hz << std::endl;
        if ( check_error( hackrf_set_sample_rate( m_device, fs_hz ) ) )
        {
            m_fs_hz = fs_hz;
            return true;
        }
    }
    return false;
}

bool HackRFDevice::set_lna_gain( uint32_t lna_gain )
{
    if ( m_is_initialized )
    {
        switch ( lna_gain )
        {
        case 0:
        case 8:
        case 16:
        case 24:
        case 32:
        case 40:
            std::cout << "Setting LNA gain..." << std::endl;
            if ( check_error( hackrf_set_lna_gain( m_device, lna_gain ) ) )
            {
                m_lna_gain = lna_gain;
                return true;
            }
        default:
            std::cout << "WARNING: invalid LNA gain: " << lna_gain << std::endl;
        }
    }
    return false;
}

bool HackRFDevice::set_rxvga_gain( uint32_t rxvga_gain )
{
    if ( m_is_initialized )
    {
        if ( ( rxvga_gain >= 0 ) && ( rxvga_gain <= 62 ) && ( rxvga_gain % 2 ) == 0 )
        {
            std::cout << "Setting Rx VGA gain..." << std::endl;
            if ( check_error( hackrf_set_vga_gain( m_device, rxvga_gain ) ) )
            {
                m_rxvga_gain = rxvga_gain;
                return true;
            }
        }
        else
            std::cout << "WARNING: invalid Rx VGA gain: " << rxvga_gain << std::endl;
    }
    return false;
}

bool HackRFDevice::set_txvga_gain( uint32_t txvga_gain )
{
    if ( m_is_initialized )
    {
        if ( txvga_gain >= 0 && txvga_gain <= 47 )
        {
            std::cout << "Setting Tx VGA gain..." << std::endl;
            if ( check_error( hackrf_set_txvga_gain( m_device, txvga_gain ) ) )
            {
                m_txvga_gain = txvga_gain;
                return true;
            }
        }
        else
            std::cout << "WARNING: invalid Tx VGA gain: " << txvga_gain << std::endl;
    }
    return false;
}

bool HackRFDevice::set_amp_enable( uint8_t amp_enable )
{
    if ( m_is_initialized )
    {
        std::cout << "Setting AMP enable..." << std::endl;
        if ( check_error( hackrf_set_amp_enable( m_device, amp_enable ) ) )
        {
            m_amp_enable = amp_enable;
            return true;
        }
    }
    return false;
}

bool HackRFDevice::set_antenna_enable( uint8_t antenna_enable )
{
    if ( m_is_initialized )
    {
        std::cout << "Setting antenna enable..." << std::endl;
        if ( check_error( hackrf_set_antenna_enable( m_device, antenna_enable ) ) )
        {
            m_antenna_enable = antenna_enable;
            return true;
        }
    }
    return false;
}


bool HackRFDevice::cleanup()
{
    if ( m_is_initialized )
    {
        m_is_initialized = false;
        
        stop_Rx();
        stop_Tx();
        
        if ( ! check_error( hackrf_close( m_device ) ) )
            std::cout << "Error: failed to release device!" << std::endl;

        hackrf_exit();
        std::cout << "Cleanup complete!" << std::endl;
        return true;
    }
    return false;
}


bool HackRFDevice::check_error(int error)
{
    hackrf_error hrf_e = hackrf_error(error);

    if ( HACKRF_SUCCESS == hrf_e )
        return true;
    else
        std::cout << "Error: " << hackrf_error_name( hrf_e ) << std::endl;
    return false;
}


