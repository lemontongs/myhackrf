
typedef int SignalID;

class SignalSaver
{
public:
    SignalSaver();
    ~SignalSaver();
    
    bool initialize();
    bool cleanup();
    
    SignalID add_signal( uint8_t * data, int num_samples );
    
    void get_signal( SignalID id );
    
    
private:
    
    struct Signal
    {
        Signal( uint8_t * data, int num_samples )
        {
            m_data = new uint8_t[ num_samples ];
            
            for (int ii = 0; ii < num_samples; ii++ )
                m_data[ii] = data[ii];
        }
        
        ~Signal()
        {
            delete [] m_data;
        }
        
        uint8_t * m_data;
    }
    
    Signal* m_signals;
    
    
};




























