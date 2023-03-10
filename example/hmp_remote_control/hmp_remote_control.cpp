#include <iostream>
#include <string>
#include <vector>

#include <labdev/tcpip_interface.hh>
#include <labdev/devices/rohde-schwarz/hmp4000.hh>

using namespace labdev;

int main () {
    printf("Starting remote control... ");
    fflush(stdout);

    // Turn on TCPIP on the HMP with MENU->Interface->Select Interface->LAN
    // You can get the IP address from MENU->Interface->Settings->IP Settings
    tcpip_interface comm("10.32.113.221", 5025);
    hmp4000 psu(&comm);

    printf("Done!\n");

    const int nch = 4;
    bool run = true;
    std::string command("");
    while (run) {

        // Print menu
        printf("======================================\n");
        printf(" S ... set output voltage\n");
        printf(" C ... toogle channel\n");
        printf(" E ... enable outputs\n");
        printf(" D ... disable outputs\n");
        printf(" R ... refresh readings\n");
        printf(" Q ... quit\n");
        printf("======================================\n");
        printf("Enter command: ");
        fflush(stdout);
        std::getline(std::cin, command);

        // Evaluate command
        if (command == "Q") {
            // Quit!
            break;
        } else if (command == "S") {
            printf("Enter channel number: ");
            fflush(stdout);
            std::getline(std::cin, command);
            unsigned channel = std::stoi(command);

            printf("Enter voltage: ");
            fflush(stdout);
            std::getline(std::cin, command);
            double volts = std::stof(command);

            psu.set_voltage(channel, volts);

        } else if (command == "C") {
            printf("Enter channel number: ");
            fflush(stdout);
            std::getline(std::cin, command);
            unsigned channel = std::stoi(command);

            if ( psu.channel_enabled(channel) )
                psu.enable_channel(channel, false);
            else
                psu.enable_channel(channel, true);

        } else if (command == "E") {
            psu.enable_outputs(true);
        } else if (command == "D") {
            psu.enable_outputs(false);
        } else if (command == "R" ) {
            // Refresh and print readings
            printf("======================================\n");
            for (int i = 1; i < nch+1; i++)
                printf(" CH%i[%s]\t%.3f\t%.3f\t%.3f\n", i,
                    psu.channel_enabled(i)? "ON" : "OFF",
                    psu.get_voltage(i),
                    psu.measure_voltage(i),
                    psu.measure_current(i) );
        } else
            printf("Invalid command!\n");
    }

    return 0;
}