
#include <rtl-sdr.h>
#include <pthread.h>

#define STANDBY_MODE 0
#define RX_MODE      1

void* thread_func(void* args);

class RTLSDRDevice
{
public:
    RTLSDRDevice();
    ~RTLSDRDevice();
    
    bool initialize();
    bool cleanup();
    
    bool start_Rx( rtlsdr_read_async_cb_t callback, void* args = NULL );
    bool stop_Rx();
    
    bool tune( uint64_t fc_hz );
    bool set_sample_rate( uint32_t fs_hz );
    bool set_lna_gain( uint32_t lna_gain );
    
    uint64_t get_center_frequency() { return m_fc_hz; };
    uint32_t get_sample_rate()      { return m_fs_hz; };
    uint32_t get_lna_gain()         { return m_lna_gain; };
    
private:
    
    // PRIVATE MEMBERS
    bool           m_is_initialized;
    int            m_device_mode;    // mode: 0=standby 1=Rx 2=Tx
    rtlsdr_dev_t*  m_device;         // device handle
    uint64_t       m_fc_hz;          // center freq
    uint32_t       m_fs_hz;          // sample rate
    uint32_t       m_lna_gain;
    pthread_t      m_thread_context; // Rx thread context
    
    rtlsdr_read_async_cb_t m_callback;
    void* m_callback_args;
    
    
    // PRIVATE FUNCTIONS
    bool check_error(int error);
    
    friend void* thread_func(void* args);
};




























