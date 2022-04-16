
#include <iostream>

#include "HackRFDevice.h"

bool g_pause_data_flow = false;

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
    m_rx_gain        = 0;
    m_tx_gain        = 0;
}

HackRFDevice::~HackRFDevice()
{
    cleanup();
}

int  HackRFDevice::get_mode()
{
    return m_device_mode;
}

bool HackRFDevice::initialize()
{
    return initialize(nullptr);
}

bool HackRFDevice::initialize(const char* const desired_serial_number)
{
    uint8_t board_id = BOARD_ID_INVALID;
    char version[255 + 1];
    
    if ( ! check_error( hackrf_init() ) )
        return false;

    if ( nullptr == desired_serial_number )
    {
        if ( ! check_error( hackrf_open( &m_device ) ) )
        {
            hackrf_exit();
            return false;
        }
    }
    else
    {
        if ( ! check_error( hackrf_open_by_serial( desired_serial_number, &m_device ) ) )
        {
            hackrf_exit();
            return false;
        }
    }
    
    std::cout << "HackRFDevice: Found HackRF board" << std::endl;

    if ( ! check_error( hackrf_board_id_read( m_device, &board_id ) ) )
        std::cout << "HackRFDevice: ERROR: reading board ID!" << std::endl;

    if ( ! check_error( hackrf_version_string_read( m_device, &version[0], 255) ) )
        std::cout << "HackRFDevice: ERROR: reading board version!" << std::endl;
    else
        std::cout << "HackRFDevice: Firmware Version: " << version << std::endl;


    m_is_initialized = true;
    
    //
    // Set device parameters
    //
    if ( ! set_center_freq( m_fc_hz ) ||
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

int convert_to_device_rx_sample_block_cb_fn(hackrf_transfer *transfer)
{
    if ( g_pause_data_flow )
        return 0;

    // printf("call\n");
    // printf("p %p\n", (void*)(transfer->rx_ctx));

    std::pair<device_sample_block_cb_fn, void*>* callback_args_pair = (std::pair<device_sample_block_cb_fn, void*>*)(transfer->rx_ctx);

    // printf("p %p\n", (void*)callback_args_pair);
    // printf("c %p\n", callback_args_pair->first);
    // printf("a %p\n", callback_args_pair->second);

    device_sample_block_cb_fn callback_function = callback_args_pair->first;
    void* callback_args = callback_args_pair->second;

    SampleChunk sc;
    sc.reserve(transfer->valid_length/2);

    for (int ii = 0; ii < transfer->valid_length; ii+=2)
    {
        sc.push_back( std::complex<double>( ( double( int8_t( transfer->buffer[ii+0] ) ) / double(128) ),
                                            ( double( int8_t( transfer->buffer[ii+1] ) ) / double(128) ) ) );
    }
    
    return callback_function(&sc, callback_args);
}

bool HackRFDevice::start_Rx( device_sample_block_cb_fn callback, void* args = NULL )
{
    if ( m_is_initialized && m_device_mode == STANDBY_MODE )
    {
        std::cout << "HackRFDevice: Starting Rx" << std::endl;

        std::pair<device_sample_block_cb_fn, void*>* callback_args = new std::pair<device_sample_block_cb_fn, void*>(callback, args);

        // printf("p %p\n", (void*)callback_args);
        // printf("c %p\n", callback);
        // printf("a %p\n", args);
    
        if ( check_error( hackrf_start_rx( m_device, convert_to_device_rx_sample_block_cb_fn, (void*)callback_args ) ) )
        {
            m_device_mode = RX_MODE;
            return true;
        }
    }
    return false;
}

bool HackRFDevice::stop_Rx()
{
    if ( m_is_initialized && m_device_mode == RX_MODE )
    {
        std::cout << "HackRFDevice: Stopping Rx" << std::endl;
        if ( ! check_error( hackrf_stop_rx( m_device ) ) )
        {
            std::cout << "HackRFDevice: Error: failed to stop rx mode!" << std::endl;
            return false;
        }
        m_device_mode = STANDBY_MODE;
        return true;
    }
    return false;
}

int convert_to_device_tx_sample_block_cb_fn(hackrf_transfer *transfer)
{
    if ( g_pause_data_flow )
        return 0;

    auto callback_args_pair = (std::pair<device_sample_block_cb_fn, void*>*)(transfer->tx_ctx);
    device_sample_block_cb_fn callback_function = callback_args_pair->first;
    void* callback_args = callback_args_pair->second;

    // Get samples from the user by calling the provided callback function
    SampleChunk sc(std::size_t(transfer->buffer_length/2));
    callback_function(&sc, callback_args);
    //std::cout << "Got " << sc.size() << " samples for Tx" << std::endl;

    for (std::size_t ii = 0; ii < sc.size(); ii++)
    {
        int8_t i = int8_t(sc[ii].real() * 127);
        int8_t q = int8_t(sc[ii].imag() * 127);
        transfer->buffer[(ii*2)+0] = uint8_t(i);
        transfer->buffer[(ii*2)+1] = uint8_t(q);
    }

    return 0;
}

bool HackRFDevice::start_Tx( device_sample_block_cb_fn callback, void* args = NULL )
{
    if ( m_is_initialized && m_device_mode == STANDBY_MODE )
    {
        std::cout << "HackRFDevice: Starting Tx" << std::endl;

        std::pair<device_sample_block_cb_fn, void*>* callback_args = new std::pair<device_sample_block_cb_fn, void*>(callback, args);

        if ( check_error( hackrf_start_tx( m_device, convert_to_device_tx_sample_block_cb_fn, (void*)callback_args ) ) )
        {
            m_device_mode = TX_MODE;
            return true;
        }
    }
    return false;
}

bool HackRFDevice::stop_Tx()
{
    if ( m_is_initialized && m_device_mode == TX_MODE )
    {
        g_pause_data_flow = true;
        std::cout << "HackRFDevice: Stopping Tx" << std::endl;
        if ( ! check_error( hackrf_stop_tx( m_device ) ) )
        {
            std::cout << "HackRFDevice: Error: failed to stop tx mode!" << std::endl;
            g_pause_data_flow = false;
            return false;
        }
        m_device_mode = STANDBY_MODE;
        g_pause_data_flow = false;
        return true;
    }
    return false;
}

bool HackRFDevice::set_center_freq( double fc_hz )
{
    if ( m_is_initialized )
    {
        if ( fc_hz >= 20000000 && fc_hz <= 6000000000 )
        {
            g_pause_data_flow = true;
            std::cout << "HackRFDevice: Tuning to " << fc_hz << std::endl;
            if ( check_error( hackrf_set_freq( m_device, fc_hz ) ) )
            {
                std::cout << "HackRFDevice: Done" << std::endl;
                m_fc_hz = fc_hz;
                g_pause_data_flow = false;
                return true;
            }
            g_pause_data_flow = false;
        }
        else
        {
            std::cout << "HackRFDevice: WARNING: invalid tune frequency: " << fc_hz << std::endl;
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
        case  2000000 :
        case  1000000 :
        {
            g_pause_data_flow = true;
            bool rv = force_sample_rate( fs_hz );
            g_pause_data_flow = false;
            return rv;
        }
        default:
            std::cout << "HackRFDevice: WARNING: invalid sample rate: " << fs_hz << std::endl;
        }
        g_pause_data_flow = false;
    }
    return false;
}

bool HackRFDevice::force_sample_rate( double fs_hz )
{
    if ( m_is_initialized )
    {
        uint32_t baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt( uint32_t(fs_hz) );
        std::cout << "HackRFDevice: Setting filter to " << baseband_filter_bw_hz << std::endl;
        hackrf_set_baseband_filter_bandwidth( m_device, baseband_filter_bw_hz );

        std::cout << "HackRFDevice: Setting sample rate to " << fs_hz << std::endl;
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
            std::cout << "HackRFDevice: LNA gain: " << lna_gain << std::endl;
            if ( check_error( hackrf_set_lna_gain( m_device, lna_gain ) ) )
            {
                m_lna_gain = lna_gain;
                return true;
            }
        default:
            std::cout << "HackRFDevice: WARNING: invalid LNA gain: " << lna_gain << std::endl;
        }
    }
    return false;
}
bool HackRFDevice::set_rx_gain( double rx_gain )
{
    return set_rxvga_gain( uint32_t(rx_gain) );
}

bool HackRFDevice::set_tx_gain( double tx_gain )
{
    return set_txvga_gain( uint32_t(tx_gain) );
}

bool HackRFDevice::set_rxvga_gain( uint32_t rxvga_gain )
{
    if ( m_is_initialized )
    {
        if ( ( rxvga_gain >= 0 ) && ( rxvga_gain <= 62 ) && ( rxvga_gain % 2 ) == 0 )
        {
            std::cout << "HackRFDevice: Setting Rx VGA gain..." << std::endl;
            if ( check_error( hackrf_set_vga_gain( m_device, rxvga_gain ) ) )
            {
                m_rxvga_gain = rxvga_gain;
                m_rx_gain = double(m_rxvga_gain);
                return true;
            }
        }
        else
            std::cout << "HackRFDevice: WARNING: invalid Rx VGA gain: " << rxvga_gain << std::endl;
    }
    return false;
}

bool HackRFDevice::set_txvga_gain( uint32_t txvga_gain )
{
    if ( m_is_initialized )
    {
        if ( txvga_gain >= 0 && txvga_gain <= 47 )
        {
            std::cout << "HackRFDevice: Setting Tx VGA gain: " << txvga_gain << std::endl;
            if ( check_error( hackrf_set_txvga_gain( m_device, txvga_gain ) ) )
            {
                m_txvga_gain = txvga_gain;
                m_tx_gain = double(m_txvga_gain);
                return true;
            }
        }
        else
            std::cout << "HackRFDevice: WARNING: invalid Tx VGA gain: " << txvga_gain << std::endl;
    }
    return false;
}

bool HackRFDevice::set_amp_enable( uint8_t amp_enable )
{
    if ( m_is_initialized )
    {
        std::cout << "HackRFDevice: Setting AMP enable: " << int(amp_enable) << std::endl;
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
        std::cout << "HackRFDevice: Setting antenna enable: " << antenna_enable << std::endl;
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
            std::cout << "HackRFDevice: Error: failed to release device!" << std::endl;

        hackrf_exit();
        std::cout << "HackRFDevice: Cleanup complete!" << std::endl;
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
        std::cout << "HackRFDevice: Error: " << hackrf_error_name( hrf_e ) << std::endl;
    return false;
}


