
#include <iostream>

#include "RTLSDRDevice.h"

RTLSDRDevice::RTLSDRDevice()
{
    m_is_initialized = false;
    m_device_mode    = STANDBY_MODE;    // mode: 0=standby 1=Rx 2=Tx
    m_fc_hz          = 462610000;       // center freq
    m_fs_hz          = 1000000;         // sample rate
    m_lna_gain       = 0;
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
    if ( ! tune( m_fc_hz ) ||
         ! set_sample_rate( m_fs_hz ) ||
         ! set_lna_gain( m_lna_gain ) )
    {
        cleanup();
        return false;
    }
        
    m_is_initialized = true;
    return true;
}

bool RTLSDRDevice::start_Rx( rtlsdr_read_async_cb_t callback, void* args = NULL )
{
    if ( m_is_initialized && m_device_mode == STANDBY_MODE )
    {
        std::cout << "Starting Rx" << std::endl;
        
        check_error( rtlsdr_reset_buffer( m_device ) );
        
        if ( check_error( rtlsdr_wait_async( m_device, callback, args ) ) )
        {
            m_device_mode == RX_MODE;
            return true;
        }
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
            return false;
        }
        m_device_mode == STANDBY_MODE;
        return true;
    }
    return false;
}

bool RTLSDRDevice::tune( uint64_t fc_hz )
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

bool RTLSDRDevice::set_sample_rate( uint32_t fs_hz )
{
    if ( m_is_initialized )
    {
        // 225001 - 300000 Hz
        // 900001 - 3200000 Hz
        if ( ( fs_hz > 225000 && fs_hz <= 300000 ) ||
             ( fs_hz > 900000 && fs_hz <= 3200000 ) )
        {
            std::cout << "Setting sample rate to " << fs_hz << std::endl;
            if ( check_error( rtlsdr_set_sample_rate( m_device, fs_hz ) ) )
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

bool RTLSDRDevice::set_lna_gain( uint32_t lna_gain )
{
    if ( m_is_initialized )
    {
        int num_gains = rtlsdr_get_tuner_gains( m_device, NULL );
        
        int* gains = new int[num_gains];
        
        rtlsdr_get_tuner_gains( m_device, gains );
        
        bool found = false;
        for (int gg = 0; gg < num_gains; gg++)
        {
            if ( gains[gg] == lna_gain )
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
        m_is_initialized = false;
        
        stop_Rx();
        
        if ( ! check_error( rtlsdr_close( m_device ) ) )
            std::cout << "Error: failed to release device!" << std::endl;

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


