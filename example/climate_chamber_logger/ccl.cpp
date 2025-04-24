#include <iostream>
#include <unistd.h>
#include <string>

#include <labdev/serial_port.hh>
#include <labdev/devices/uni-t/ut61b.hh>

using namespace std;
using namespace labdev;

int main(int argc, char** argv)
{
    // Path to device file of DMM
    string dmm_path = "/dev/ttyUSB1";
    ut61b dmm(make_unique<serial_port>(dmm_path, 2400));
    cout << dmm.get_value() << dmm.get_unit() << endl;
/*
    for (unsigned i = 0; i < 10; i++) 
    {
        cout << dmm.get_value() << dmm.get_unit() << endl;
        sleep(1);
    }
*/
    return 0;
}