
#include <rtl-sdr.h>
#include <pthread.h>

#include "RFDevice.h"

#define STANDBY_MODE 0
#define RX_MODE      1

void* thread_func(void* args);

class RTLSDRDevice : public RFDevice
{
public:
    RTLSDRDevice();
    ~RTLSDRDevice();
    
    bool initialize();
    bool cleanup();
    
    bool start_Rx( device_sample_block_cb_fn callback, void* args );
    bool stop_Rx();
    bool start_Tx( device_sample_block_cb_fn callback, void* args ) { return false; };
    bool stop_Tx() { return false; };
    
    //bool tune( uint64_t fc_hz );
    bool set_center_freq( double fc_hz );

    //bool set_sample_rate( uint32_t fs_hz );
    bool set_sample_rate( double fs_hz );

    bool set_lna_gain( uint32_t lna_gain );
    bool set_rx_gain( double rx_gain );
    bool set_tx_gain( double tx_gain ) { return false; };

    uint64_t get_center_frequency() { return m_fc_hz; };
    uint32_t get_sample_rate()      { return m_fs_hz; };
    uint32_t get_rx_gain()          { return m_rx_gain; };
    
private:
    
    // PRIVATE MEMBERS
    int            m_device_mode;    // mode: 0=standby 1=Rx 2=Tx
    rtlsdr_dev_t*  m_device;         // device handle
    pthread_t      m_thread_context; // Rx thread context
    
    std::pair<device_sample_block_cb_fn, void*>* m_callback_args;
    
    // PRIVATE FUNCTIONS
    bool check_error(int error);
    
    friend void* thread_func(void* args);
};




























