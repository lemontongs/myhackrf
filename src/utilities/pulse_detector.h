
#ifndef PULSE_DETECTOR_H
#define PULSE_DETECTOR_H

#include "packet.pb.h"

#include "fft.h"

#include <stdint.h>
#include <vector>
#include <deque>
#include <fstream>
#include <sstream>

namespace utilities
{
    class MovingAverage
    {
    public:
        MovingAverage(const uint32_t window_size) :
            m_window(),
            m_sum(0.0),
            m_max_size(window_size)
        {}

        double addItem(double item)
        {
            if ( isinf(item) )
                return addItem(0.0);
            
            m_window.push_front(item);
            m_sum += item;

            if ( m_window.size() > m_max_size )
            {
                m_sum -= m_window[m_window.size()-1];
                m_window.pop_back();
            }

            return getAverage();
        }

        double getAverage()
        {
            if ( m_window.size() > 0 )
                return m_sum / m_window.size();
            else
                return 0.0;
        }

        double getMax()
        {
            double max = -9999;
            for ( auto val : m_window )
            {
                max = std::max(max, val);
            }
            return max;
        }

        void clear()
        {
            m_window.clear();
            m_sum = 0.0;
        }

        std::deque<double> m_window;
        double m_sum;
        uint32_t m_max_size;
    };
    
    enum DetectionState
    {
        NOISE,
        DETECTION
    };

    struct LogItem
    {
        double index;
        double i;
        double q;
        double amp;
        double state;
        double det_count;
        double noise_average;
        double num_pdws;
    };
    typedef std::vector<LogItem> LogItemList;

    //
    // Find pulses in IQ data
    //
    void detectPulses(PDW_Packet* result, 
                      SampleChunk* samples, 
                      uint64_t base_sample_count, 
                      double fs, 
                      double threshold_over_mean_db)
    {
        static bool log = true;
        LogItemList log_data;
        
        // For each sample
        double max_iq_db = -INFINITY;
        double sum_iq_db = 0.0;
        std::vector<double> sample_power(std::size_t(samples->size()));

        for (std::size_t ii = 0; ii < samples->size(); ii++)
        {
            // Calc power
            double iq_db = 20.0f * std::log10( std::abs( (*samples).at(ii) ) );

            // Store amplitude for pulse detection
            sample_power[ii] = iq_db;
            
            if ( ! isinf(iq_db) )
            {
                // Calc max power
                max_iq_db = std::max(max_iq_db, iq_db);
            
                // Store sum for mean
                sum_iq_db += iq_db;
            }
        }

        double mean_iq_db = sum_iq_db / double(samples->size());

        // Rising edge = M of N samples over threshold
        // Falling edge = M of N samples below threshold
        double pulse_declaration_threshold = (mean_iq_db + threshold_over_mean_db);
        const double M_s = 0.000012;
        const double N_s = 0.000015;
        const uint32_t M = uint32_t(M_s * fs);
        const uint32_t N = uint32_t(N_s * fs);

        // Loop sample power
        uint32_t det_count = 0;
        DetectionState state = NOISE;
        uint32_t toa = 0;
        uint32_t pw = 0;
        MovingAverage noise_average(sample_power.size());
        log_data.reserve(sample_power.size());
        
        for (std::size_t ii = 0; ii < sample_power.size(); ii++)
        {
            // Check for sample detection
            if ( sample_power[ii] > pulse_declaration_threshold )
            {
                det_count++;
            }

            // Decrement det count more than N samples ago
            if ( ii >= N && sample_power[ii-N] > pulse_declaration_threshold )
            {
                det_count--;
            }

            // Detector state machine
            if ( state == NOISE )
            {
                // Rising edge
                if ( det_count > M )
                {
                    state = DETECTION;
                    toa = ii - det_count;
                }
                else
                {
                    noise_average.addItem(sample_power[ii]);
                }
            }
            
            if ( state == DETECTION )
            {
                // Falling edge
                if ( det_count < M )
                {
                    state = NOISE;
                    
                    // Find the real pulse edges, rather than the M of N boundaries
                    for (std::size_t jj = toa; jj < ii; jj++)
                    {
                        if ( sample_power[jj] > pulse_declaration_threshold )
                        {
                            toa = jj;
                            break;
                        }
                    }
                    for (std::size_t jj = (ii-(2*M)); jj < ii; jj++)
                    {
                        if ( sample_power[jj] < pulse_declaration_threshold )
                        {
                            pw = jj - toa;
                            break;
                        }
                    }

                    PDW* new_pdw = result->add_pdws();
                    new_pdw->set_toa_s( double(base_sample_count + toa) / fs );
                    new_pdw->set_pw_s( double(pw) / fs );

                    // Compute center frequency
                    FFT_Packet result;
                    SampleChunk buffer;
                    for (std::size_t jj = 0; jj < pw; jj++)
                        buffer.push_back((*samples).at(toa + jj));

                    fft( &result, buffer, 512, fs, 0.0 );
                    double max_val = -9999999.0;
                    std::size_t max_ind = 0;
                    for (int jj = 0; jj < result.fft_size(); jj++)
                    {
                        if ( result.fft(jj) > max_val )
                        {
                            max_val = result.fft(jj);
                            max_ind = jj;
                        }
                    }
                    new_pdw->set_freq_offset_hz( result.freq_bins_hz(max_ind) );

                    // Measure average power
                    MovingAverage pulse_average(pw);
                    for (std::size_t jj = 0; jj < pw; jj++)
                    {
                        new_pdw->add_signal((*samples).at(toa + jj).real());
                        new_pdw->add_signal((*samples).at(toa + jj).imag());
                        pulse_average.addItem(sample_power[toa + jj]);
                    }

                    new_pdw->set_mean_amp_db(pulse_average.getAverage());
                    new_pdw->set_peak_amp_db(pulse_average.getMax());
                    new_pdw->set_noise_amp_db(noise_average.getAverage());
                }
            }
            LogItem it;
            it.index = ii;
            it.i = (*samples).at(ii).real();
            it.q = (*samples).at(ii).imag();
            it.amp = sample_power[ii];
            it.state = state;
            it.det_count = det_count;
            it.noise_average = noise_average.getAverage();
            it.num_pdws = result->pdws_size();
            
            log_data.push_back(it);
        }

        
        if (log && result->pdws_size() > 0 && result->pdws()[0].signal().size() > 20)
        {
            std::ofstream log_stream;
            log_stream.open("detector.log", std::ofstream::out);
            log_stream << 
                    "index" << "," << 
                    "i" << "," << 
                    "q" << "," <<
                    "amp" << "," <<
                    "state" << "," <<
                    "det_count" << "," <<
                    "noise_average" << "," <<
                    "num_pdws" << std::endl;
            
            for (auto it : log_data)
            {
                log_stream << 
                    it.index << "," << 
                    it.i << "," << 
                    it.q << "," <<
                    it.amp << "," <<
                    it.state << "," <<
                    it.det_count << "," <<
                    it.noise_average << "," <<
                    it.num_pdws << std::endl;
            }

            std::cout << "PULSE LOG!" << std::endl;

            log = false;
        }
        
        // if (result->pdws_size() > 0)
        //     std::cout << "PULSES: " << result->pdws_size() << "  MEAN: " << mean_iq_db << "  PEAK: " << max_iq_db << std::endl;
    }
}

#endif /* PULSE_DETECTOR_H */

