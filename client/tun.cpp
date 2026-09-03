#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include <sys/select.h>
#include <algorithm>

/*

Your computer
     │
     │ TCP/IP packet
     ▼
Google

                Chrome
                ↓
                Linux TCP/IP stack
                ↓                            Chrome
                ↓                            ↓
                ↓                            Linux TCP/IP stack
                ↓                            ↓
                ↓                            tun0
                ↓                            ↓
                ↓                            YOUR C++ APPLICATION
                ↓                            ↓
                ↓                            OUR TUNNEL
                ↓                            ↓
                ↓                            network path
                wlo1
                ↓
                Wi-Fi
                ↓
                Internet

*/

int main() {

    // 1. Open the TUN device
    int tun_fd = open("/dev/net/tun", O_RDWR);

    if (tun_fd < 0) {
        perror("open /dev/net/tun");
        return 1;
    }

    // 2. Describe the virtual interface

    /*
    ifreq
    │
    ├── interface name
    │
    ├── interface flags
    │
    └── other interface configuration information

    */
    struct ifreq ifr{};
    
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    // Ask Linux to create tun0
    std::strcpy(ifr.ifr_name, "tun0");

    // 3. Create the interface
    if (ioctl(tun_fd, TUNSETIFF, &ifr) < 0) {
        perror("TUNSETIFF");
        close(tun_fd);
        return 1;
    }

    std::cout << "TUN interface created: "
              << ifr.ifr_name
              << std::endl;
    

    // =========================================================
    // 2. Create UDP socket
    // =========================================================

    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_sock < 0) {
        perror("socket");
        close(tun_fd);
        return 1;
    }

    // =========================================================
    // 3. Describe Docker server
    // =========================================================

    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);

    // Docker container IP
    inet_pton(
        AF_INET,
        "127.0.0.1",
        // "172.17.0.2",
        &serverAddr.sin_addr
    );



    // =========================================================
    // 4. Read packets from TUN
    // =========================================================

    char buffer[2000];
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);

    // while (true) {

    //     ssize_t bytes = read(
    //         tun_fd,
    //         buffer,
    //         sizeof(buffer)
    //     );

    //     if (bytes < 0) {
    //         perror("read tun0");
    //         break;
    //     }

    //     std::cout
    //         << "TUN packet received: "
    //         << bytes
    //         << " bytes"
    //         << std::endl;


    //     // =====================================================
    //     // 5. Send the REAL IP packet through UDP
    //     // =====================================================

    //     ssize_t sent = sendto(
    //         udp_sock,
    //         buffer,
    //         bytes,
    //         0,
    //         reinterpret_cast<sockaddr*>(&serverAddr),
    //         sizeof(serverAddr)
    //     );

    //     if (sent < 0) {
    //         perror("sendto");
    //         break;
    //     }

    //     std::cout
    //         << "Sent through UDP: "
    //         << sent
    //         << " bytes"
    //         << std::endl;
    // }

    while (true) {

        fd_set readfds;

        FD_ZERO(&readfds);

        FD_SET(tun_fd, &readfds);
        FD_SET(udp_sock, &readfds);

        int max_fd = std::max(tun_fd, udp_sock);

        int ready = select(
            max_fd + 1,
            &readfds,
            nullptr,
            nullptr,
            nullptr
        );

        if (ready < 0) {
            perror("select");
            break;
        }


        // =====================================================
        // HOST TUN → UDP → DOCKER
        // =====================================================

        if (FD_ISSET(tun_fd, &readfds)) {

            ssize_t bytes = read(
                tun_fd,
                buffer,
                sizeof(buffer)
            );

            if (bytes < 0) {
                perror("read tun0");
                break;
            }

            std::cout
                << "HOST TUN -> UDP: "
                << bytes
                << " bytes"
                << std::endl;

            ssize_t sent = sendto(
                udp_sock,
                buffer,
                bytes,
                0,
                reinterpret_cast<sockaddr*>(&serverAddr),
                sizeof(serverAddr)
            );

            if (sent < 0) {
                perror("sendto");
                break;
            }

            std::cout
                << "Sent to Docker: "
                << sent
                << " bytes"
                << std::endl;
        }


        // =====================================================
        // DOCKER UDP → HOST TUN
        // =====================================================

        if (FD_ISSET(udp_sock, &readfds)) {

            sockaddr_in fromAddr{};
            socklen_t fromLen = sizeof(fromAddr);

            ssize_t bytes = recvfrom(
                udp_sock,
                buffer,
                sizeof(buffer),
                0,
                reinterpret_cast<sockaddr*>(&fromAddr),
                &fromLen
            );

            if (bytes < 0) {
                perror("recvfrom");
                break;
            }

            std::cout
                << "UDP -> HOST TUN: "
                << bytes
                << " bytes"
                << std::endl;

            ssize_t written = write(
                tun_fd,
                buffer,
                bytes
            );

            if (written < 0) {
                perror("write tun0");
                break;
            }

            std::cout
                << "Written into host TUN: "
                << written
                << " bytes"
                << std::endl;
        }
    }
    close(udp_sock);
    close(tun_fd);

    return 0;
}

/*
                     YOUR COMPUTER
┌────────────────────────────────────────────────────┐
│                                                    │
│                    Linux kernel                    │
│                                                    │
│   Application ──► TCP/IP stack ──► tun0           │
│                                      │             │
│                                      │ IP packet   │
│                                      ▼             │
│                              ┌───────────────┐     │
│                              │ Your C++      │     │
│                              │ tunnel client │     │
│                              └───────┬───────┘     │
│                                      │             │
│                              UDP / encrypted       │
│                                      │             │
└──────────────────────────────────────┼─────────────┘
                                       │
                                       ▼
                              Docker container
                         ┌───────────────────────┐
                         │   tunnel server       │
                         │                       │
                         │ UDP socket :5000      │
                         │       │               │
                         │       ▼               │
                         │     tun0              │
                         └───────┬───────────────┘
                                 │
                                 ▼
                           Linux networking
*/