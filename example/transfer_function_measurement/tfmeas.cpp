#include <unistd.h>
#include <iostream>
#include <cmath>
#include <fstream>

#include <labdev/tcpip_interface.hh>
#include <labdev/usbtmc_interface.hh>
#include <labdev/devices/rigol/dg4000.hh>
#include <labdev/devices/rigol/ds1000z.hh>


using namespace labdev;

void dso_setup(ds1000z* dso, bool reset = false);
void fgen_setup(dg4000* fgen, bool reset = false);

int main (int argc, char** argv) {
    // You can get the IP addresses from the menu "Utility->IO Setting"
    // Port 5555 is default for SCPI
    tcpip_interface comm_dso("192.168.2.10", 5555);
    tcpip_interface comm_fgen("192.168.2.11", 5555);
    //usbtmc_interface comm_dso(ds1000z::DS1104_VID, ds1000z::DS1104_PID);
    //usbtmc_interface comm_fgen(dg4000::DG4162_VID, dg4000::DG4162_PID);

    ds1000z dso(&comm_dso);
    dso_setup(&dso);

    dg4000 fgen(&comm_fgen);
    fgen_setup(&fgen);

    // Logarithmic frequency sweep
    const int npts = 20;
    const double fsta = 1e3;    // 1kHz
    const double fsto = 10e6;   // 10MHz
    std::vector<double> fset;
    for (int i = 0; i < npts; i++) {
        double freq = pow(10, log10(fsto/fsta)*i/(npts-1) + log10(fsta) );
        fset.push_back(freq);
    }

    // Data storage
    double a1_avg, a1_std, a2_avg, a2_std, ph_avg, ph_std;
    std::ofstream out_file("measurement.csv", std::ofstream::out);
    out_file << "Freq [Hz], A1 [V], STD1[V], A2 [V], STD2[V], Phase [deg], STD Phase[deg]\n";

    for (const auto& f : fset) {

        try {
            // Set new frequency
            fgen.set_freq(1, f);
            fgen.wait_to_complete();
            // CHange timescale according to frequency and reset measurements
            dso.set_horz_base(1./f);
            dso.reset_measurements();
            dso.wait_to_complete();

            sleep(1);   // Time to gather statistics

            // Read measurements
            a1_avg = dso.get_measurement(1, ds1000z::MEAS_VAMP, ds1000z::MEAS_AVG);
            a1_std = dso.get_measurement(1, ds1000z::MEAS_VAMP, ds1000z::MEAS_STD);
            a2_avg = dso.get_measurement(2, ds1000z::MEAS_VAMP, ds1000z::MEAS_AVG);
            a2_std = dso.get_measurement(2, ds1000z::MEAS_VAMP, ds1000z::MEAS_STD);
            ph_avg = dso.get_measurement(1, 2, ds1000z::MEAS_RPH, ds1000z::MEAS_AVG);
            ph_std = dso.get_measurement(1, 2, ds1000z::MEAS_RPH, ds1000z::MEAS_STD);
        } catch (const exception& ex) {
            // Save measurement and stop program
            std::cout << "Measurement failed (" << ex.what() << ")!" << std::endl;
            out_file.close();
            throw;
        }

        printf("%.1f Hz\t(%.3f ± %.3f) V\t(%.3f ± %.3f) V\t(%.3f ± %.3f) deg\n",
            f, a1_avg, a1_std, a2_avg, a2_std, ph_avg, ph_std);

        out_file << f << ","
        << a1_avg << "," << a1_std << ","
        << a2_avg << "," << a2_std << ","
        << ph_avg << "," << ph_std << "\n";
    }

    out_file.close();

    fgen.enable_channel(1, false);

    return 0;
 }

void dso_setup(ds1000z* dso, bool reset) {
    std::cout << "Preparing DSO... " << std::flush;
    if (reset) {
        dso->reset();
        sleep(4);
    }
    for (int i = 1; i < 3; i++) {
        dso->enable_channel(i);
        dso->set_atten(i, 1);
        dso->set_vert_base(i, 1.0);
        dso->set_measurement(i, ds1000z::MEAS_VAMP);
    }
    dso->set_measurement(1, 2, ds1000z::MEAS_RPH);
    dso->wait_to_complete();
    std::cout << "Done!" << std::endl;
    return;
}

void fgen_setup(dg4000* fgen, bool reset) {
    std::cout << "Preparing function generator... " << std::flush;
    if (reset) {
        fgen->reset();
        sleep(2);
    }
    fgen->enable_channel(1);
    fgen->set_waveform(1, dg4000::SINE);
    fgen->set_freq(1, 1000);
    fgen->set_ampl(1, 5.);
    fgen->set_offset(1, 0.);
    fgen->set_phase(1, 0.);

    std::cout << "Done!" << std::endl;
    return;
}