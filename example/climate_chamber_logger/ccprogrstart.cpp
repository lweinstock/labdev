#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <signal.h>

#include <labdev/devices/cts/c70_1500.hh>

using namespace std;
using namespace labdev;


int main(int argc, char** argv)
{
    // Climate chamber setup
    string chamber_ip = "192.168.1.90";
    c70_1500 chamber(make_unique<tcpip_iface>(chamber_ip, c70_1500::PORT));
    chamber.run_program(120);
}