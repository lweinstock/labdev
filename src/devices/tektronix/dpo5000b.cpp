#include <labdev/devices/tektronix/dpo5000b.hh>
#include <labdev/utils/utils.hh>
#include "ld_debug.hh"

#include <sstream>

namespace labdev {

    dpo5000b::dpo5000b():
        m_xincr(0),
        m_xzero(0),
        m_ymult(0),
        m_yzero(0),
        m_yoff(0),
        m_nbyte(0),
        m_nbits(0),
        m_npts(0) {
        return;
    }

    dpo5000b::dpo5000b(visa_interface* visa) : scpi_device(visa) {
        // VISA setup
        visa->flush_buffer();
        visa->clear_device();
        this->init();
        return;
    }

    dpo5000b::dpo5000b(tcpip_interface* tcpip) : scpi_device(tcpip) {
        this->init();
        return;
    }

    dpo5000b::dpo5000b(usbtmc_interface* usbtmc) : scpi_device(usbtmc) {
        // USBTMC I/O setup
        usbtmc->claim_interface(0);
        usbtmc->set_endpoint_in(0x01);
        usbtmc->set_endpoint_out(0x02);
        // Clear I/O buffer
        usbtmc->clear_buffer();
        this->init();
        return;
    }

    dpo5000b::~dpo5000b() {
        return;
    }

    void dpo5000b::enable_channel(unsigned channel, bool enable) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << "SEL:CH" << channel;
        if (enable) msg << " 1\n";
        else msg << " 0\n";
        comm->write(msg.str());
        return;
    }

    void dpo5000b::start_acquisition() {
        comm->write("ACQ:STATE RUN\n");
        return;
    }
    void dpo5000b::stop_acquisition() {
        comm->write("ACQ:STATE STOP\n");
        return;
    }

    void dpo5000b::set_edge_trigger(unsigned channel, double level, uint8_t edge) {
        this->check_channel(channel);
        // Check edge
        std::string edge_str("");
        switch (edge) {
            case RISE:
                edge_str = "RIS";
                break;
            case FALL:
                edge_str = "FALL";
                break;
            case BOTH:
                edge_str = "EIT";
                break;

            default:
            fprintf(stderr, "Invalid edge %i\n", edge);
            abort();
        }
        // Set trigger source
        std::stringstream msg("");
        msg << "TRIG:A:EDGE:SOU CH" << channel << "\n";
        comm->write( msg.str() );

        // Set edge trigger slope
        msg.str("");
        msg << "TRIG:A:EDGE:SLO " << edge_str << "\n";
        comm->write( msg.str() );

        // Set trigger level
        msg.str("");
        msg << "TRIG:A:LEV " << level << "\n";
        comm->write( msg.str() );

        return;
    }

    bool dpo5000b::triggered() {
        std::string msg = comm->query("TRIG:STATE?\n");
        if (msg.find("TRIGGER") == std::string::npos)
            return false;
        else
            return true;
    }

    void dpo5000b::set_sample_rate(int samples_per_sec) {
        std::stringstream msg("");
        msg << "HOR:MODE:SAMPLER " << samples_per_sec << "\n";
        comm->write( msg.str() );
        return;
    }

    int dpo5000b::get_sample_rate() {
        int sample_rate = -1;
        std::string msg = comm->query("HOR:MODE:SAMPLER?\n");
        if ( !msg.empty() )
            sample_rate = std::stoi(msg);
        return sample_rate;
    }

    void dpo5000b::set_sample_len(int rec_len) {
        std::stringstream msg("");
        // Set record length
        msg << "HOR:MODE:RECO " << rec_len << "\n";
        comm->write( msg.str() );
        // Set sample start and stop
        msg.str("");
        msg << "DAT:STOP  " << rec_len << "\n";
        comm->write("DAT:STAR 1\n");
        comm->write( msg.str() );
        return;
    }

    int dpo5000b::get_sample_len() {
        int sample_len = -1;
        std::string msg = comm->query("HOR:MODE:RECO?\n");
        if ( !msg.empty() )
            sample_len = std::stoi(msg);
        return sample_len;
    }

    void dpo5000b::sample_screen() {
        comm->write("DAT SNA\n");
        return;
    }

    void dpo5000b::read_sample_data(unsigned channel,
        std::vector<double> &horz_data, std::vector<double> &vert_data) {
        this->check_channel(channel);
        // Set channel as source
        comm->write(":DAT:SOU CH" + std::to_string(channel) + "\n");

        // Clear vectors
        horz_data.clear();
        vert_data.clear();

        // Read preamble
        std::string data = comm->query("WFMO?\n");
        std::vector<std::string> preamble = split(data, ";");
        if (preamble.size() != 16) {
            debug_print("Received wrong preamble: %s\n", data.c_str());
            throw bad_protocol("Received incomplete preamble.\n", -1);
        }

        // Extract data from preamble
        m_nbyte   = std::stoi( preamble.at(0) );
        m_nbits   = std::stoi( preamble.at(1) );
        m_npts    = std::stoi( preamble.at(6) );
        m_xincr   = std::stof( preamble.at(9) );
        m_xzero   = std::stof( preamble.at(10) );
        m_xoff    = std::stof( preamble.at(11) );
        m_ymult   = std::stof( preamble.at(13) );
        m_yzero   = std::stof( preamble.at(14) );
        m_yoff    = std::stof( preamble.at(15) );

        debug_print("nbyte = %i\n", m_nbyte);
        debug_print("nbits = %i\n", m_nbits);
        debug_print("npts = %i\n", m_npts);
        debug_print("ymult = %e\n", m_ymult);
        debug_print("yzero = %e\n", m_yzero);
        debug_print("yoff = %e\n", m_yoff);
        debug_print("xincr = %e\n", m_xincr);
        debug_print("xzero = %e\n", m_xzero);

        // Get waveform
        data.clear();
        data = comm->query("CURV?\n", 5000);
        // Remove header
        int hdr_len = 2 + std::stoi( data.substr(1,1) );
        int npts = data.size() - hdr_len - 1;
        // Read until all points ar read
        while (npts < m_npts) {
            data.append( comm->read() );
            npts = data.size() - hdr_len - 1;
            debug_print("Number of points read %i\n", npts);
        }

        double xval, yval;
        for (int i = 0; i < npts; i++) {
            xval = (i - m_xoff) * m_xincr + m_xzero;
            yval = ((int8_t)data.at(i + hdr_len) - m_yzero) * m_ymult + m_yoff;
            vert_data.push_back(xval);
            horz_data.push_back(yval);
        }
        return;
    }

    void dpo5000b::set_vert_base(unsigned channel, double volts_per_div) {
        this->check_channel(channel);
        std::stringstream msg;
        msg << "CH" << channel << ":SCA " << volts_per_div << "\n";
        comm->write( msg.str() );
        return;
    }

    double dpo5000b::get_vert_base(unsigned channel) {
        this->check_channel(channel);

        double volts_per_div = -1;
        std::stringstream msg;
        msg << "CH" << channel << ":SCA?" << "\n";
        std::string resp = comm->query( msg.str() );
        if ( !resp.empty() )
            volts_per_div = std::stof(resp);
        return volts_per_div;
    }

    void dpo5000b::set_horz_base(double sec_per_div) {
        std::stringstream msg;
        msg << "HOR:MODE:SCA " << sec_per_div << "\n";
        comm->write( msg.str() );
        return;
    }

    double dpo5000b::get_horz_base() {
        double sec_per_div = -1;
        std::string msg = comm->query("HOR:MODE:SCA?\n");
        if ( !msg.empty() )
            sec_per_div = std::stof(msg);
        return sec_per_div;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void dpo5000b::init() {
        m_xincr = 0;
        m_xzero = 0;
        m_xoff  = 0;
        m_ymult = 0;
        m_yzero = 0;
        m_yoff  = 0;
        m_nbyte = 0;
        m_nbits = 0;
        m_npts  = 0;

        this->clear_status();
        // Capture data shown on screen
        this->sample_screen();
        // Data setup:
        //   Sample data binary encoded, 1 byte per sample, 8 bits per byte,
        //   MSB first, signed integer format, screen snapshot
        comm->write("WFMO:ENC BIN\n");
        comm->write("WFMO:BYT_N 1\n");
        comm->write("WFMO:BIT_N 8\n");
        comm->write("WFMO:BYT_O MSB\n");
        comm->write("WFMO:BNF RI\n");
        comm->write("DAT SNA\n");
        this->wait_to_complete();
        return;
    }

    void dpo5000b::check_channel(unsigned channel) {
        // Check channel
        if (channel > 4) {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        return;
    }

}