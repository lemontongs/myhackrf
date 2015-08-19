
#ifndef HACKRFDEVICE_H
#define HACKRFDEVICE_H


#include <hackrf.h>

#define STANDBY_MODE 0
#define RX_MODE      1
#define TX_MODE      2

class HackRFDevice
{
public:
    HackRFDevice();
    ~HackRFDevice();
    
    bool initialize();
    bool cleanup();
    
    bool start_Rx( hackrf_sample_block_cb_fn callback, void* args );
    bool stop_Rx();
    bool start_Tx( hackrf_sample_block_cb_fn callback, void* args );
    bool stop_Tx();
    
    bool tune( uint64_t fc_hz );
    bool force_sample_rate( double fs_hz ); // not recommended
    bool set_sample_rate( double fs_hz );
    bool set_lna_gain( uint32_t lna_gain );
    bool set_amp_enable( uint8_t amp_enable );
    bool set_antenna_enable( uint8_t value );
    bool set_rxvga_gain( uint32_t rxvga_gain );
    bool set_txvga_gain( uint32_t txvga_gain );
    
    uint64_t get_center_frequency() { return m_fc_hz; };
    double   get_sample_rate()      { return m_fs_hz; };
    uint32_t get_lna_gain()         { return m_lna_gain; };
    uint8_t  get_amp_enable()       { return m_amp_enable; };
    uint8_t  get_antenna_enable()   { return m_antenna_enable; };
    uint32_t get_rxvga_gain()       { return m_rxvga_gain; };
    uint32_t get_txvga_gain()       { return m_txvga_gain; };
    
private:
    
    // PRIVATE MEMBERS
    bool           m_is_initialized;
    int            m_device_mode;    // mode: 0=standby 1=Rx 2=Tx
    hackrf_device* m_device;         // device handle
    uint64_t       m_fc_hz;          // center freq
    double         m_fs_hz;          // sample rate
    uint32_t       m_lna_gain;
    uint8_t        m_amp_enable;
    uint8_t        m_antenna_enable;
    uint32_t       m_rxvga_gain;
    uint32_t       m_txvga_gain;
    
    // PRIVATE FUNCTIONS
    bool check_error(int error);
};



#endif /*  HACKRFDEVICE */
























