#include <labdev/utils/waveform.hh>

#include <iostream>
#include <cmath>

using namespace std;

namespace labdev {

waveform::waveform(double* xData, double* yData, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        m_xval.push_back(xData[i]);
        m_yval.push_back(yData[i]);
    }
    return;
}

waveform::waveform(vector<double> xData, vector<double> yData)
{
    if ( xData.size() != yData.size() ) {
        cerr << "X and Y vectors have to be same size" << endl;
        abort();
    }
    m_xval = xData;
    m_yval = yData;
    return;
}

void waveform::add_point(double x, double y)
{
    m_xval.push_back(x);
    m_yval.push_back(y);
    return;
}

void waveform::insert_point(unsigned idx, double x, double y)
{
    m_xval.insert(m_xval.begin() + idx, x);
    m_yval.insert(m_yval.begin() + idx, y);
    return;
}

void waveform::clear() 
{ 
    m_xval.clear(); 
    m_yval.clear(); 
    return;
}

double waveform::get_mean(unsigned sta, unsigned sto)
{
    this->check_sta_sto(sta, sto);
    double mean = .0;
    for (size_t i = sta; i < sto; i++)
        mean += m_yval.at(i);
    return mean/(sto - sta);
}

double waveform::get_rms(unsigned sta, unsigned sto)
{
    this->check_sta_sto(sta, sto);
    double rms = .0;
    for (size_t i = sta; i < sto; i++)
        rms += m_yval.at(i) * m_yval.at(i);
    rms /= (sto - sta);
    return sqrt(rms);
}

double waveform::get_stdev(unsigned sta, unsigned sto)
{
    this->check_sta_sto(sta, sto);
    double mean = .0, rms = .0;
    for (size_t i = sta; i < sto; i++) 
    {
        mean += m_yval.at(i);
        rms += m_yval.at(i) * m_yval.at(i);
    }
    mean /=(sto - sta);
    rms /= (sto - sta);
    return (rms - mean*mean);
}

void waveform::get_fft(waveform &magn, waveform &phase)
{
    unsigned nPts = m_xval.size();
    unsigned nFFT = nPts/2 + 1;
    double dx = abs(m_xval.at(0) - m_xval.at(1));
    fftw_complex* fft = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nFFT);
    fftw_plan plan = fftw_plan_dft_r2c_1d(nPts, m_yval.data(), fft, FFTW_ESTIMATE);
    fftw_execute(plan);

    magn.clear();
    phase.clear();
    for (unsigned i = 0; i < nFFT; i++) 
    {
        double f = i/(dx * nPts);
        double m = sqrt( fft[i][0]*fft[i][0] + fft[i][1]*fft[i][1] )/nPts;
        double ph = atan(fft[i][1]/fft[i][0]);
        magn.add_point(f, m);
        phase.add_point(f, ph);
    }

    fftw_destroy_plan(plan);
    fftw_free(fft);
    return;
}

void waveform::apply_filter(filter_function filter)
{
    unsigned nPts = m_xval.size();
    unsigned nFFT = nPts/2 + 1;
    double dx = abs(m_xval.at(0) - m_xval.at(1));
    fftw_complex* fft = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nFFT);
    fftw_plan plan = fftw_plan_dft_r2c_1d(nPts, m_yval.data(), fft, FFTW_ESTIMATE);
    fftw_execute(plan);

    // Apply filter
    for (unsigned i = 0; i < nFFT; i++) 
    {
        double f = i/(dx * nPts);
        fft[i][0] *= filter(f)/nPts;
        fft[i][1] *= filter(f)/nPts;
    }

    // Inverse transform
    plan = fftw_plan_dft_c2r_1d(nPts, fft, m_yval.data(), FFTW_ESTIMATE);
    fftw_execute(plan);

    fftw_destroy_plan(plan);
    fftw_free(fft);   
    
    return;
}

/*
 *      S T A T I C   F I L T E R   M E T H O D S
 */

waveform::filter_function waveform::gaus(double amplitude, double mean, double sigma) 
{
    return [amplitude, mean, sigma](double x) -> double {
        return amplitude * exp(-0.5 * (x - mean)*(x - mean)/(sigma * sigma));
    };
}

waveform::filter_function waveform::butterworth_lowpass(double gain, double fc, unsigned n)
{
    return [gain, fc, n](double x) -> double {
        return gain/sqrt(1 + pow((x/fc), 2*n));
    };
}

waveform::filter_function waveform::butterworth_highpass(double gain, double fc, unsigned n)
{
    return [gain, fc, n](double x) -> double {
        return gain/sqrt(1 + pow((fc/x), 2*n));
    };
}

waveform::filter_function waveform::butterworth_bandpass(double gain, double f1, double f2,  unsigned n)
{
    return [gain, f1, f2, n](double x) -> double {
        double f0 = sqrt(f2*f1);
        double a = sqrt(f2/f1);
        return gain/sqrt(1 + pow( (x/f0 - f0/x)/a, 2*n));
    };
}

/*
 *      P R I V A T E   M E T H O D S
 */

void waveform::check_sta_sto(unsigned sta, unsigned sto) {
    if ( (sta < this->get_size()) && (sto < this->get_size()) )
        return;
    std::cerr << "Invalid start and stop values: " << sta << " - " << sto << std::endl;
    abort();
    return;
}

}
