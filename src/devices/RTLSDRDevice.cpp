
#include <iostream>

#include "RTLSDRDevice.h"

RTLSDRDevice::RTLSDRDevice()
{
    m_is_initialized = false;
    m_device_mode    = STANDBY_MODE;    // mode: 0=standby 1=Rx 2=Tx
    m_fc_hz          = 462610000;       // center freq
    m_fs_hz          = 1000000;         // sample rate
    m_rx_gain        = 0;
}

RTLSDRDevice::~RTLSDRDevice()
{
    cleanup();
}

bool RTLSDRDevice::initialize()
{
    if ( 0 >= rtlsdr_get_device_count() )
    {
        std::cout << "No device" << std::endl;
        return false;
    }

    if ( ! check_error( rtlsdr_open( &m_device, 0 ) ) )
    {
        std::cout << "Failed to open device" << std::endl;
        return false;
    }

    std::cout << "Found rtlsdr board" << std::endl;

    m_is_initialized = true;
    
    //
    // Set device parameters
    //
    if ( ! set_center_freq( m_fc_hz ) ||
         ! set_sample_rate( m_fs_hz ) ||
         ! set_rx_gain( m_rx_gain ) )
    {
        cleanup();
        return false;
    }
        
    m_is_initialized = true;
    return true;
}

//
// Callback function for Rx samples
//
void rx_cb_fn(unsigned char *buf, uint32_t len, void *ctx)
{
    std::cout << "got a packet! (" << len << ")" << std::endl;
    
    std::pair<device_sample_block_cb_fn, void*>* callback_args_pair = (std::pair<device_sample_block_cb_fn, void*>*)ctx;

    device_sample_block_cb_fn callback_function = callback_args_pair->first;
    void* callback_args = callback_args_pair->second;

    SampleChunk sc;
    sc.resize(len);

    for (int ii = 0; ii < len; ii+=2)
    {
        sc[ii] = std::complex<double>( ( double( int8_t( buf[ii+0] ) ) / double(128) ),
                                       ( double( int8_t( buf[ii+1] ) ) / double(128) ) );
    }

    callback_function(&sc, callback_args);
}

void* thread_func(void* args)
{
    std::cout << "Rx thread started" << std::endl;
    
    RTLSDRDevice* rtl_obj = (RTLSDRDevice*)args;
    
    rtl_obj->m_device_mode = RX_MODE;
    
    rtl_obj->check_error( rtlsdr_reset_buffer( rtl_obj->m_device ) );
    
    rtl_obj->check_error( 
        rtlsdr_read_async( 
            rtl_obj->m_device, 
            rx_cb_fn,
            rtl_obj->m_callback_args, 
            0, 
            0 ) ); // Blocking
    
    return nullptr;
}

bool RTLSDRDevice::start_Rx( device_sample_block_cb_fn callback, void* args )
{
    if ( m_is_initialized && m_device_mode == STANDBY_MODE )
    {
        std::cout << "Starting Rx thread " << m_is_initialized << std::endl;
        
        m_callback_args = new std::pair<device_sample_block_cb_fn, void*>(callback, args);
        
        pthread_create( &m_thread_context, NULL, thread_func, (void*)this );
        
        return true;
    }
    return false;
}

bool RTLSDRDevice::stop_Rx()
{
    if ( m_is_initialized && m_device_mode == RX_MODE )
    {
        std::cout << "Stopping Rx" << std::endl;
        if ( ! check_error( rtlsdr_cancel_async( m_device ) ) )
        {
            std::cout << "Error: failed to stop rx mode!" << std::endl;
            delete m_callback_args;
            return false;
        }
        pthread_join( m_thread_context, NULL );
        m_device_mode = STANDBY_MODE;
        delete m_callback_args;
        return true;
    }
    return false;
}

bool RTLSDRDevice::set_center_freq( double fc_hz )
{
    if ( m_is_initialized )
    {
        if ( fc_hz >= 20000000 && fc_hz <= 6000000000 )
        {
            std::cout << "Tuning to " << fc_hz << std::endl;
            if ( check_error( rtlsdr_set_center_freq( m_device, fc_hz ) ) )
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

bool RTLSDRDevice::set_sample_rate( double fs_hz )
{
    if ( m_is_initialized )
    {
        // 225001 - 300000 Hz
        // 900001 - 3200000 Hz
        if ( ( fs_hz > 225000 && fs_hz <= 300000 ) ||
             ( fs_hz > 900000 && fs_hz <= 3200000 ) )
        {
            std::cout << "Setting sample rate to " << fs_hz << std::endl;
            if ( check_error( rtlsdr_set_sample_rate( m_device, uint32_t(fs_hz) ) ) )
            {
                m_fs_hz = fs_hz;
                return true;
            }
        }
        else
            std::cout << "WARNING: invalid sample rate: " << fs_hz << std::endl;
    }
    return false;
}

bool RTLSDRDevice::set_rx_gain( double lna_gain )
{
    if ( m_is_initialized )
    {
        int num_gains = rtlsdr_get_tuner_gains( m_device, NULL );
        
        int* gains = new int[num_gains];
        
        rtlsdr_get_tuner_gains( m_device, gains );
        
        for (int gg = 0; gg < num_gains; gg++)
        {
            if ( gains[gg] == int(lna_gain) )
            {
                std::cout << "Setting LNA gain..." << std::endl;
                rtlsdr_set_tuner_gain( m_device, lna_gain );
                delete [] gains;
                return true;
            }
        }
        
        std::cout << "Invalid LNA gain, valid values are";
        for (int gg = 0; gg < num_gains; gg++)
            std::cout << ", " << gains[gg];
        std::cout << std::endl;
        
        delete [] gains;
    }
    return false;
}

bool RTLSDRDevice::cleanup()
{
    if ( m_is_initialized )
    {
        stop_Rx();
        
        if ( ! check_error( rtlsdr_close( m_device ) ) )
            std::cout << "Error: failed to release device!" << std::endl;

        m_is_initialized = false;
        
        std::cout << "Cleanup complete!" << std::endl;
        return true;
    }
    return false;
}


bool RTLSDRDevice::check_error(int error)
{
    if ( 0 == error )
        return true;
    else
        std::cout << "Error: " << error << std::endl;
    return false;
}


