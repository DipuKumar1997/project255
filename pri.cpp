#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

const string WIFI = "wlo1";
const string ETHERNET = "enxd6798406c37d";

// How often to check Wi-Fi
const int CHECK_INTERVAL_SECONDS = 3;

// Number of consecutive failures before switching
const int FAILURE_LIMIT = 3;

// Google's DNS server
const char* TEST_IP = "8.8.8.8";
const int TEST_PORT = 53;


// ------------------------------------------------------------
// Run shell command
// ------------------------------------------------------------

bool runCommand(const string& command)
{
    cout << "[CMD] " << command << '\n';

    int result = system(command.c_str());

    return result == 0;
}


// ------------------------------------------------------------
// Check whether interface exists
// ------------------------------------------------------------

bool interfaceExists(const string& iface)
{
    string command =
        "ip link show " + iface + " >/dev/null 2>&1";

    return system(command.c_str()) == 0;
}


// ------------------------------------------------------------
// Set route metric using NetworkManager
// ------------------------------------------------------------

void setRouteMetrics()
{
    cout << "\n[+] Setting route priorities...\n";

    // Lower metric = higher priority.
    //
    // Wi-Fi      = 100
    // Ethernet   = 600

    runCommand(
        "nmcli device modify " + WIFI +
        " ipv4.route-metric 100"
    );

    runCommand(
        "nmcli device modify " + ETHERNET +
        " ipv4.route-metric 600"
    );

    cout << "[+] Wi-Fi metric    = 100\n";
    cout << "[+] Ethernet metric = 600\n\n";
}


// ------------------------------------------------------------
// Check Internet specifically through Wi-Fi
// ------------------------------------------------------------
//
// IMPORTANT:
//
// We bind the socket to wlo1.
//
// Therefore, even if Ethernet has working Internet,
// this function will fail when Wi-Fi itself has lost Internet.
//
// ------------------------------------------------------------

bool wifiInternetWorks()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        cerr << "[ERROR] socket(): "
             << strerror(errno) << '\n';

        return false;
    }


    // Force this socket to use Wi-Fi interface.
    //
    // Linux-specific socket option.
    //

    if (setsockopt(
            sock,
            SOL_SOCKET,
            SO_BINDTODEVICE,
            WIFI.c_str(),
            WIFI.size() + 1) < 0)
    {
        cerr << "[ERROR] SO_BINDTODEVICE: "
             << strerror(errno) << '\n';

        close(sock);
        return false;
    }


    // Give the connection a timeout.
    struct timeval timeout;

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_SNDTIMEO,
        &timeout,
        sizeof(timeout)
    );

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout)
    );


    // Destination
    sockaddr_in server{};

    server.sin_family = AF_INET;
    server.sin_port = htons(TEST_PORT);

    if (inet_pton(
            AF_INET,
            TEST_IP,
            &server.sin_addr) <= 0)
    {
        close(sock);
        return false;
    }


    cout << "[CHECK] Testing Internet through "
         << WIFI << "...\n";


    int result = connect(
        sock,
        reinterpret_cast<sockaddr*>(&server),
        sizeof(server)
    );


    close(sock);


    if (result == 0)
    {
        cout << "[CHECK] Wi-Fi Internet = OK\n";
        return true;
    }


    cout << "[CHECK] Wi-Fi Internet = FAILED\n";

    return false;
}


// ------------------------------------------------------------
// Disconnect Wi-Fi
// ------------------------------------------------------------

void disableWifi()
{
    cout << "\n========================================\n";
    cout << "[FAILOVER] Wi-Fi Internet lost!\n";
    cout << "[FAILOVER] Disconnecting Wi-Fi...\n";
    cout << "========================================\n\n";

    runCommand(
        "nmcli device disconnect " + WIFI
    );

    cout << "[FAILOVER] Ethernet should now be active.\n\n";
}


// ------------------------------------------------------------
// Reconnect Wi-Fi
// ------------------------------------------------------------

void enableWifi()
{
    cout << "\n========================================\n";
    cout << "[RECOVERY] Trying to reconnect Wi-Fi...\n";
    cout << "========================================\n\n";

    runCommand(
        "nmcli device connect " + WIFI
    );

    // Give DHCP a little time.
    this_thread::sleep_for(
        chrono::seconds(3)
    );

    cout << "[RECOVERY] Testing Wi-Fi again...\n";

    if (wifiInternetWorks())
    {
        cout << "[RECOVERY] Wi-Fi Internet restored.\n";
        cout << "[RECOVERY] Wi-Fi is primary again.\n\n";
    }
    else
    {
        cout << "[RECOVERY] Wi-Fi still has no Internet.\n";
        cout << "[RECOVERY] Ethernet remains the backup.\n\n";
    }
}


// ------------------------------------------------------------
// Show routing table
// ------------------------------------------------------------

void showRoutes()
{
    cout << "\n========== CURRENT ROUTES ==========\n";

    system("ip route");

    cout << "====================================\n\n";
}


// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main()
{
    cout << "========================================\n";
    cout << " Linux Wi-Fi -> Ethernet Failover\n";
    cout << "========================================\n\n";


    // Check interfaces
    if (!interfaceExists(WIFI))
    {
        cerr << "[ERROR] Wi-Fi interface "
             << WIFI << " does not exist.\n";

        return 1;
    }

    if (!interfaceExists(ETHERNET))
    {
        cerr << "[ERROR] Ethernet interface "
             << ETHERNET << " does not exist.\n";

        return 1;
    }


    cout << "[+] Wi-Fi interface: "
         << WIFI << '\n';

    cout << "[+] Ethernet interface: "
         << ETHERNET << "\n\n";


    // Set priorities
    setRouteMetrics();

    showRoutes();


    bool wifiConnected = true;
    int failures = 0;


    while (true)
    {
        // ----------------------------------------------------
        // Wi-Fi currently active
        // ----------------------------------------------------

        if (wifiConnected)
        {
            bool ok = wifiInternetWorks();


            if (ok)
            {
                failures = 0;
            }
            else
            {
                failures++;

                cout << "[WARNING] Wi-Fi failure "
                     << failures << "/"
                     << FAILURE_LIMIT << '\n';


                if (failures >= FAILURE_LIMIT)
                {
                    disableWifi();

                    wifiConnected = false;

                    failures = 0;

                    showRoutes();
                }
            }
        }


        // ----------------------------------------------------
        // Wi-Fi currently disconnected
        // ----------------------------------------------------

        else
        {
            cout << "\n[BACKUP] Ethernet is currently handling traffic.\n";

            cout << "[BACKUP] Checking whether Wi-Fi can recover...\n";


            enableWifi();


            // Check whether Wi-Fi really recovered.
            //

            if (wifiInternetWorks())
            {
                wifiConnected = true;

                failures = 0;

                // Make sure Wi-Fi remains preferred.
                setRouteMetrics();

                showRoutes();
            }
            else
            {
                // Wi-Fi failed again.
                //
                // Disconnect it again so Ethernet
                // remains the active Internet route.

                cout << "[BACKUP] Wi-Fi recovery failed.\n";
                cout << "[BACKUP] Returning to Ethernet backup.\n";

                runCommand(
                    "nmcli device disconnect " + WIFI
                );
            }
        }


        this_thread::sleep_for(
            chrono::seconds(CHECK_INTERVAL_SECONDS)
        );
    }


    return 0;
}