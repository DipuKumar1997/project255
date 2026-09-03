#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>



#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>





#include <sys/select.h>
#include <algorithm>

#include <poll.h>

#include "packet.hpp"

int createTun(const char* name) {

    // int tun_fd =createTun("tun0");
    // if(tun_fd<0)return 1;


        int tun_fd = open("/dev/net/tun", O_RDWR); 
        if (tun_fd < 0) {
        perror("Opening /dev/net/tun");
        return -1;
    }


    struct ifreq ifr{};

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    strncpy(ifr.ifr_name, name, IFNAMSIZ);

    if (ioctl(tun_fd, TUNSETIFF, &ifr) < 0) {
        perror("TUNSETIFF");
        close(tun_fd);
        return -1;
    }

    std::cout << "Created TUN interface: "
              << ifr.ifr_name
              << std::endl;

    return tun_fd;
}

int main() {

    int tun_fd = createTun("tun0");

        if (tun_fd < 0) {
            return 1;
        }

    // =========================================================
    // 1. Create UDP socket
    // =========================================================

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }


    /// =========================================================
    // 2. Describe server address
    // =========================================================

    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5000);




    // =========================================================
    // 3. Bind UDP socket
    // =========================================================

    if (bind(
            sock,
            reinterpret_cast<sockaddr*>(&serverAddr),
            sizeof(serverAddr)
        ) < 0) {

        perror("bind");
        close(sock);
        return 1;
    }


    std::cout
        << "Server listening on UDP port 5000"
        << std::endl;




    
    // =========================================================
    // 4. Receive tunnel packets
    // =========================================================

    char buffer[2000];

     sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);


    bool clientKnown = false;

    // while (true) {

        // ssize_t bytes = recvfrom(
        //     sock,
        //     buffer,
        //     sizeof(buffer),
        //     0,
        //     reinterpret_cast<sockaddr*>(&clientAddr),
        //     &clientLen
        // );
        // char buffer[2000];

    //     ssize_t bytes = recvfrom(
    //         sock,
    //         buffer,
    //         sizeof(buffer),
    //         0,
    //         reinterpret_cast<sockaddr*>(&clientAddr),
    //         &clientLen
    //     );

    //     if (bytes < 0) {
    //         perror("recvfrom");
    //         break;
    //     }

    //     ssize_t written = write(tun_fd, buffer, bytes);

    //     if (written < 0) {
    //         perror("write tun");
    //     } else {
    //         std::cout << "UDP -> TUN: "
    //                 << written
    //                 << " bytes"
    //                 << std::endl;
    //     }

    //     std::cout
    //         << "Tunnel packet received: "
    //         << bytes
    //         << " bytes"
    //         << std::endl;


    //     // Print first few bytes in hexadecimal.
    //     std::cout << "First bytes: ";

    //     int show = std::min<ssize_t>(bytes, 16);

    //     for (int i = 0; i < show; ++i) {

    //         printf(
    //             "%02x ",
    //             static_cast<unsigned char>(buffer[i])
    //         );
    //     }

    //     std::cout << std::endl;
    // }


    while (true) {

        fd_set readfds;

        FD_ZERO(&readfds);

        FD_SET(tun_fd, &readfds);
        FD_SET(sock, &readfds);

        int max_fd = std::max(tun_fd, sock);

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
        // UDP → TUN
        // =====================================================

        if (FD_ISSET(sock, &readfds)) {

            ssize_t bytes = recvfrom(
                sock,
                buffer,
                sizeof(buffer),
                0,
                reinterpret_cast<sockaddr*>(&clientAddr),
                &clientLen
            );

            if (bytes < 0) {
                perror("recvfrom");
                break;
            }

            clientKnown = true;

            std::cout
                << "UDP -> TUN: "
                << bytes
                << " bytes"
                << std::endl;

            ssize_t written = write(
                tun_fd,
                buffer,
                bytes
            );

            if (written < 0) {
                perror("write tun");
            }
        }


        // =====================================================
        // TUN → UDP
        // =====================================================

        if (FD_ISSET(tun_fd, &readfds)) {

            ssize_t bytes = read(
                tun_fd,
                buffer,
                sizeof(buffer)
            );

            if (bytes < 0) {
                perror("read tun");
                break;
            }

            std::cout
                << "TUN -> UDP: "
                << bytes
                << " bytes"
                << std::endl;

             if (!clientKnown) {

                std::cout
                    << "Client address not known yet - dropping TUN packet"
                    << std::endl;

                continue;
            }

            ssize_t sent = sendto(
                sock,
                buffer,
                bytes,
                0,
                reinterpret_cast<sockaddr*>(&clientAddr),
                clientLen
            );

            if (sent < 0) {
                perror("sendto");
            } else {
                std::cout
                    << "Sent back through UDP: "
                    << sent
                    << " bytes"
                    << std::endl;
            }
        }
    }




    // // std::cout << "Server listening on UDP port 5000\n";
    // std::cout << "Server listening on UDP port 5000" << std::endl;

    // // 4. Receive packets
    // // char buffer[1024];
    // Packet packet{};


    // sockaddr_in clientAddr{};
    // socklen_t clientLen = sizeof(clientAddr);

    // while (true) {

    //         ssize_t bytes = recvfrom(
    //             sock,
    //             &packet,
    //             sizeof(packet),
    //             0,
    //             reinterpret_cast<sockaddr*>(&clientAddr),
    //             &clientLen
    //         );

    //         if (bytes < 0) {
    //             perror("recvfrom");
    //             break;
    //         }

    //         // buffer[bytes] = '\0';

    //         // std::cout << "Received: "
    //         //           << buffer
    //         //           << '\n';

    //         std::cout << "Packet received"<<std::endl;
    //         std::cout << "  connection: " << packet.connection_id << std::endl;
    //         std::cout << "  sequence:   " << packet.sequence << std::endl;
    //         std::cout << "  data:       " << packet.data << std::endl;

    //         // 5. Send response back
    //     //     const char* response = "ACK";

    //     //     sendto(
    //     //         sock,
    //     //         response,
    //     //         strlen(response),
    //     //         0,
    //     //         reinterpret_cast<sockaddr*>(&clientAddr),
    //     //         clientLen
    //     //     );
    //     // }
    //     Packet ack{};

    //     ack.connection_id = packet.connection_id;
    //     ack.sequence = packet.sequence;

    //     std::strcpy(ack.data, "ACK");

    //     ssize_t sent = sendto(
    //         sock,
    //         &ack,
    //         sizeof(ack),
    //         0,
    //         reinterpret_cast<sockaddr*>(&clientAddr),
    //         clientLen
    //     );

    //     if (sent < 0) {
    //         perror("sendto");
    //     } else {
    //         std::cout << "ACK sent for sequence: "
    //                 << ack.sequence
    //                 << std::endl;
    //     }

    // }

    close(sock);
    return 0;

}


/*
              REAL PACKET
                   │
                   ▼
                tun0
                   │
                 read()
                   │
                   ▼
              C++ client
                   │
                sendto()
                   │
                   ▼
              UDP :5000
                   │
                   ▼
             Docker server
                   │
              recvfrom()
                   ▼
             REAL PACKET
*/