
#ifndef HACKRFDEVICE_H
#define HACKRFDEVICE_H

#include "RFDevice.h"

#include <libhackrf/hackrf.h>

class HackRFDevice : public RFDevice
{
public:
    HackRFDevice();
    ~HackRFDevice();
    
    bool initialize();
    bool initialize(const char* const desired_serial_number);
    bool cleanup();
    
    bool start_Rx( device_sample_block_cb_fn callback, void* args );
    bool stop_Rx();
    bool start_Tx( device_sample_block_cb_fn callback, void* args );
    bool stop_Tx();
    
    int  get_mode(); // returns m_device_mode
    
    bool set_center_freq( double fc_hz );
    bool set_rx_gain( double rx_gain );
    bool set_tx_gain( double tx_gain );

    
    bool force_sample_rate( double fs_hz ); // not recommended
    bool set_sample_rate( double fs_hz );
    bool set_lna_gain( uint32_t lna_gain );
    bool set_amp_enable( uint8_t amp_enable );
    bool set_antenna_enable( uint8_t value );
    bool set_rxvga_gain( uint32_t rxvga_gain );
    bool set_txvga_gain( uint32_t txvga_gain );
    
    uint32_t get_lna_gain()         { return m_lna_gain; };
    uint8_t  get_amp_enable()       { return m_amp_enable; };
    uint8_t  get_antenna_enable()   { return m_antenna_enable; };
    uint32_t get_rxvga_gain()       { return m_rxvga_gain; };
    uint32_t get_txvga_gain()       { return m_txvga_gain; };
    
private:
    
    // PRIVATE MEMBERS
    int            m_device_mode;    // mode: 0=standby 1=Rx 2=Tx
    hackrf_device* m_device;         // device handle
    uint32_t       m_lna_gain;
    uint8_t        m_amp_enable;
    uint8_t        m_antenna_enable;
    uint32_t       m_rxvga_gain;
    uint32_t       m_txvga_gain;
    
    // PRIVATE FUNCTIONS
    bool check_error(int error);
};



#endif /*  HACKRFDEVICE */
























