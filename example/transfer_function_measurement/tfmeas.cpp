#include <unistd.h>
#include <iostream>
#include <fstream>

#include <labdev/devices/rigol/ds1000z.hh>
#include <labdev/devices/siglent/sdg1000x.hh>

using namespace std;
using namespace labdev;

int main (int argc, char** argv) 
{
    // Osci Setup
    auto comm_dso = std::make_unique<tcpip_iface>("192.168.2.101", ds1000z::PORT);
    ds1000z dso(std::move(comm_dso));
    dso.enable_channel(1);
    dso.enable_channel(2);
    dso.set_vert_base(1, 0.2);
    dso.set_vert_base(2, 0.2);

    // Function generator setup
    auto comm_fgen = std::make_unique<tcpip_iface>("192.168.2.102", sdg1000x::PORT);
    sdg1000x fgen(std::move(comm_fgen));
    fgen.set_wvfm(1, fgen::SINE);
    fgen.set_ampl(1, 1);
    fgen.set_offset(1, 0);

    // Calculate points for log sweep
    vector<float> freq;
    float fsta = 100;
    float fsto = 100e3;
    unsigned n = 20;
    float step = (log10(fsto) - log10(fsta))/(n-1);
    for (unsigned i = 0; i < n; i++) 
    {
        float f = pow(10, step * i + log10(fsta));
        freq.push_back(f);
    }

    // Start measurement
    ofstream fout("data.csv", fstream::out); 
    fgen.enable_channel(1);
    for (unsigned i = 0; i < freq.size(); ) 
    {
        float f = freq.at(i);
        fgen.set_freq(1, f);
        dso.set_horz_base(1./f);

        // wait for measurements to stabilize
        if (f == fsta)
            sleep(1);
        sleep(1);

        float vpp1 = dso.get_meas(1, osci::meas_item::VPP);
        float vpp2 = dso.get_meas(2, osci::meas_item::VPP);
        float phase = dso.get_meas(1, 2, osci::meas_item::POS_PHASE);

        float vert = dso.get_vert_base(2);  // Adjust the scale if the signal
        if ( vpp2 < 3*vert ) {              // if too small or large and 
            dso.set_vert_base(2, vert/2);   // repeate the measurement
            continue;
        } else if ( vpp2 > 7*vert) {
            dso.set_vert_base(2, 2*vert);
            continue;
        }

        fout << f/1000. << ", " << phase << ", " << vpp1 << ", " << vpp2 << endl;
        i++;
    }
    fgen.disable_channel(1);
    fout.close();

    return 0;
}