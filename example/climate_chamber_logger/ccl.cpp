#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <signal.h>

#include <labdev/exceptions.hh>
#include <labdev/serial_port.hh>
#include <labdev/devices/uni-t/ut61b.hh>
#include <labdev/devices/cts/c70_1500.hh>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace labdev;

volatile bool stop = false;

void print_usage()
{
    cout << "Usage: ./ccl [output.csv]" << endl;
    return;
}

void handler(int signal)
{
    cout << "Press 'x' to quit, any other key to continue." << endl;

    char input = getchar();
    if (input == 'x')
        stop = true;
    return;
}

int main(int argc, char** argv)
{
    if (argc != 2) 
    {
        print_usage();
        return -1;
    }

    signal(SIGINT, handler);

    // DMM setup
    string dmm_path = "/dev/ttyUSB0";
    ut61b dmm(make_unique<serial_port>(dmm_path, 2400));

    // Climate chamber setup
    string chamber_ip = "192.168.1.90";
    c70_1500 chamber(make_unique<tcpip_iface>(chamber_ip, c70_1500::PORT));

    // Camera setup
    cv::VideoCapture cap;
    if (!cap.open("/dev/video14", cv::CAP_V4L2))
    {
        cout << "Could not open camera" << endl;
        return -1;
    }

    // Output file setup
    string fname(argv[1]);
    // Add .csv-suffix, if none was provided
    if (fname.find(".csv") == string::npos)
        fname += ".csv";
    ofstream out_file(fname, ofstream::out | ofstream::app);
    if (!out_file.good())
    {
        cout << "Failed to create output file '" + fname << "'" << endl;
        return -1;
    }

    // Main measurement loop
    while (true) 
    {
        if (stop) break;
        double dmm_val = -1;
        double humidity = -1;
        double temperature = -1;

        // Create timestamp for logging
        time_t cur_time = time(nullptr);
        string time_stamp = asctime(localtime(&cur_time));
        // Remove year and date => result: "hh:mm:ss"
        time_stamp = time_stamp.erase(time_stamp.find_last_of(" "));
        time_stamp = time_stamp.erase(0, time_stamp.find_last_of(" ") + 1);

        // Try to get readings from DMM and climate chamber
        try 
        {
            dmm_val = dmm.get_value();
            temperature = chamber.get_current_temperature();
            humidity = chamber.get_current_humidity();
        }
        catch (labdev::exception &ex)   // Error!
        {
            cout << ex.what() << endl;
            break;
        }
        cout << time_stamp << " (" << cur_time << ")" << "\t";
        cout << dmm_val << ", ";
        cout << temperature << ", ";
        cout << humidity << endl;

        out_file << time_stamp << ", "; 
        out_file << dmm_val << ", ";
        out_file << temperature << ", ";
        out_file << humidity << endl;

        // Try to take picture from camera
        cv::Mat frame;
        cap.read(frame);
        if (frame.empty())  // Error!
        {
            cout << "Grabbed empty frame" << endl;
            break;
        }
        string img_name = "img_" + time_stamp + ".png";
        cv::imwrite(img_name.c_str(), frame);

        sleep(1);
    }

    out_file.close();

    return 0;
}