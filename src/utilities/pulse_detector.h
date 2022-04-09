
#ifndef PULSE_DETECTOR_H
#define PULSE_DETECTOR_H

#include "packet.pb.h"

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
    void detectPulses(PDW_Packet* result, SampleChunk* samples, uint64_t base_sample_count, double fs, double threshold_over_mean_db)
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
        const uint32_t M = 5;
        const uint32_t N = 8;

        // Loop sample power
        uint32_t det_count = 0;
        DetectionState state = NOISE;
        uint32_t toa = 0;
        uint32_t pw = 0;
        std::vector<std::complex<double>> pulse_iq;
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
                    toa = ii;
                }
                else
                {
                    noise_average.addItem(sample_power[ii]);
                }
            }
            
            if ( state == DETECTION )
            {
                // Falling edge
                if ( (N - det_count) > M )
                {
                    state = NOISE;
                    pw = ii - toa;
                    std::cout << "PULSE: " << toa << "  " << pw << std::endl;
                    PDW* new_pdw = result->add_pdws();
                    new_pdw->set_toa_s( (base_sample_count + toa) / fs );

                    //new_pdw->set_freq_offset_hz();
                    
                    MovingAverage pulse_average(pw);

                    for (std::size_t jj = 0; jj < pulse_iq.size(); jj++)
                    {
                        new_pdw->add_signal(pulse_iq[jj].real());
                        new_pdw->add_signal(pulse_iq[jj].imag());

                        pulse_average.addItem(sample_power[toa+jj]);
                    }

                    new_pdw->set_mean_amp_db(pulse_average.getAverage());
                    new_pdw->set_peak_amp_db(pulse_average.getMax());
                    new_pdw->set_noise_amp_db(noise_average.getAverage());

                    pulse_iq.clear();
                }
                else
                {
                    // Save the samples
                    pulse_iq.push_back( (*samples).at(ii) );
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
        
        
        std::cout << "PULSES: " << result->pdws_size() << "  MEAN: " << mean_iq_db << "  PEAK: " << max_iq_db << std::endl;
    }
}

#endif /* PULSE_DETECTOR_H */

