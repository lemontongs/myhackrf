
#ifndef RFDEVICE_H
#define RFDEVICE_H

#include <vector>
#include <complex>

#define STANDBY_MODE 0
#define RX_MODE      1
#define TX_MODE      2

typedef std::vector<std::complex<double>> SampleChunk;

typedef int (*device_sample_block_cb_fn)(SampleChunk* samples, void* args);

class RFDevice
{
public:
    virtual bool initialize() = 0;
    virtual bool cleanup() = 0;

    virtual bool start_Rx( device_sample_block_cb_fn callback, void* args ) = 0;
    bool stop_Rx();
    virtual bool start_Tx( device_sample_block_cb_fn callback, void* args ) = 0;
    bool stop_Tx();
    
    virtual bool set_center_freq( double fc_hz ) = 0;
    virtual bool set_sample_rate( double fs_hz ) = 0;
    virtual bool set_rx_gain( double rx_gain ) = 0;
    virtual bool set_tx_gain( double tx_gain ) = 0;
    
    double   get_center_freq() { return m_fc_hz; };
    double   get_sample_rate() { return m_fs_hz; };
    double   get_rx_gain()     { return m_rx_gain; };
    double   get_tx_gain()     { return m_tx_gain; };
    
protected:
    
    // PRIVATE MEMBERS
    bool           m_is_initialized;
    double         m_fc_hz;          // center freq
    double         m_fs_hz;          // sample rate
    double         m_rx_gain;
    double         m_tx_gain;
};



#endif /*  RFDEVICE */
