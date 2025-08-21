#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <signal.h>

#include <labdev/exceptions.hh>
#include <labdev/serial_port.hh>
#include <labdev/devices/lc_hx711/hx711_arduino.hh>

using namespace std;
using namespace labdev;

volatile bool stop = false;

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
    signal(SIGINT, handler);

    // LC setup
    string lc_path = "/dev/ttyACM1"; // serial device
    hx711_arduino lchx711;
    // Force the serial_iface overload explicitly:
    std::unique_ptr<serial_iface> ser = std::make_unique<serial_port>(lc_path, 115200);
    lchx711.connect(std::move(ser));

    sleep(1);
 
    // Main measurement loop
    while (true) 
    {
        if (stop) break;
        int lc_val = -1;

        // Try to get readings from DMM and climate chamber
        try 
        {
            lc_val = lchx711.read_lc();
        }
        
        catch (labdev::exception &ex)   // Error!
        {
            cout << ex.what() << endl;
            break;
        }

        cout << lc_val << endl;

        sleep(1);
    }


    return 0;
}