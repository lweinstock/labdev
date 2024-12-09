#ifndef LD_WAVEFORM_HH
#define LD_WAVEFORM_HH

#include <vector>
#include <functional>
#include <fftw3.h>

namespace labdev {

class waveform
{
public:
    waveform() {};
    waveform(double* xData, double* yData, size_t len);
    waveform(std::vector<double> xData, std::vector<double> yData);
    ~waveform() {};

    void add_point(double x, double y);
    void insert_point(unsigned idx, double x, double y);

    double get_x(unsigned i) { return m_xval.at(i); } 
    double get_y(unsigned i) { return m_yval.at(i); } 
    std::vector<double> get_x() { return m_xval; }
    std::vector<double> get_y() { return m_yval; }
    size_t get_size() { return m_xval.size(); }
    void clear();

    double get_mean(unsigned sta, unsigned sto);
    double get_mean() { return this->get_mean(0, this->get_size() - 1); }
    double get_rms(unsigned sta, unsigned sto);
    double get_rms() { return this->get_rms(0, this->get_size() - 1); }
    double get_stdev(unsigned sta, unsigned sto);
    double get_stdev() { return this->get_stdev(0, this->get_size() - 1); }

    void get_fft(waveform &magn, waveform &phase);

    // Static filter functions for use with apply_filter
    typedef std::function<double(double)> filter_function;
    void apply_filter(filter_function filter);
    static filter_function gaus(double amplitude, double mean, double sigma);
    static filter_function butterworth_lowpass(double gain, double fc, unsigned n);
    static filter_function butterworth_highpass(double gain, double fc, unsigned n);
    static filter_function butterworth_bandpass(double gain, double f1, double f2, unsigned n);

private:
    std::vector<double> m_xval {};
    std::vector<double> m_yval {};

    void check_sta_sto(unsigned sta, unsigned sto);
};

}

#endif