// #include <arpa/inet.h>
// #include <cstring>
// #include <iostream>
// #include <sys/socket.h>
// #include <unistd.h>

// int main() {

//     // 1. Create UDP socket
//     int sock = socket(AF_INET, SOCK_DGRAM, 0);

//     if (sock < 0) {
//         perror("socket");
//         return 1;
//     }

//     // 2. Describe server
//     sockaddr_in serverAddr{};

//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(5000);

//     inet_pton(
//         AF_INET,
//         "172.17.0.1",
//         &serverAddr.sin_addr
//     );

//     // 3. Send packet
//     const char* message = "HELLO FROM CLIENT";

//     sendto(
//         sock,
//         message,
//         strlen(message),
//         0,
//         reinterpret_cast<sockaddr*>(&serverAddr),
//         sizeof(serverAddr)
//     );

//     std::cout << "Packet sent\n";

//     // 4. Receive server response
//     char buffer[1024];

//     sockaddr_in serverResponse{};
//     socklen_t serverLen = sizeof(serverResponse);

//     ssize_t bytes = recvfrom(
//         sock,
//         buffer,
//         sizeof(buffer) - 1,
//         0,
//         reinterpret_cast<sockaddr*>(&serverResponse),
//         &serverLen
//     );

//     if (bytes >= 0) {
//         buffer[bytes] = '\0';

//         std::cout << "Server replied: "
//                   << buffer
//                   << '\n';
//     }

//     close(sock);
// }


#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#include "packet.hpp"


int main() {

    // 1. Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // 2. Describe server
    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddr.sin_addr
    );

    // 3. Send packet
    const char* message = "HELLO FROM CLIENT";

    // ssize_t sent = sendto(
    //         sock,
    //         message,
    //         strlen(message),
    //         0,
    //         reinterpret_cast<sockaddr*>(&serverAddr),
    //         sizeof(serverAddr)
    //     );


    // for (uint32_t seq = 1; seq <= 10; ++seq) {

    //     Packet packet{};

    //     packet.sequence = seq;
    //     packet.connection_id = 1001;

    //     std::snprintf(  packet.data,  sizeof(packet.data),  "HELLO PACKET %u",  seq );

    //     ssize_t sent = sendto( sock,  &packet,  sizeof(packet),  0,  reinterpret_cast<sockaddr*>(&serverAddr),  sizeof(serverAddr) );

    //     if (sent < 0) {
    //         perror("sendto");
    //         break;
    //     }

    //     std::cout << "Sent packet "  << packet.sequence   << std::endl;
    // }

    for (uint32_t seq = 1; seq <= 10; ++seq) {

        Packet packet{};

        packet.connection_id = 1001;
        packet.sequence = seq;

        std::snprintf(  packet.data,  sizeof(packet.data),  "HELLO PACKET %u",  seq );

        // Send
        ssize_t sent = sendto(    sock,   &packet,   sizeof(packet),    0,    reinterpret_cast<sockaddr*>(&serverAddr),    sizeof(serverAddr) );

        if (sent < 0) {
            perror("sendto");
            break;
        }

        std::cout << "Sent sequence: "    << packet.sequence    << std::endl;


        // Receive ACK
        Packet ack{};

        sockaddr_in serverResponse{};
        socklen_t serverLen = sizeof(serverResponse);

        ssize_t bytes = recvfrom(  sock,  &ack,  sizeof(ack),  0,  reinterpret_cast<sockaddr*>(&serverResponse),  &serverLen );

        if (bytes < 0) {
            perror("recvfrom");
            break;
        }

        std::cout << "ACK received\n";
        std::cout << "  connection: " << ack.connection_id << std::endl;
        std::cout << "  sequence:   " << ack.sequence << std::endl;
        std::cout << "  data:       " << ack.data << std::endl;
    }
    // Packet packet{};

    // packet.sequence = 1;
    // packet.connection_id = 1001;

    // std::strcpy(packet.data, "HELLO FROM CLIENT");

    // ssize_t sent = sendto(
    //     sock,
    //     &packet,
    //     sizeof(packet),
    //     0,
    //     reinterpret_cast<sockaddr*>(&serverAddr),
    //     sizeof(serverAddr)
    // );

    //     if (sent == -1) {
    //         perror("sendto");
    //         close(sock);
    //         return 1;
    //     }

    //     std::cout << "sendto() returned: "
    //             << sent
    //             << " bytes\n"<<std::endl;

    // if (sent < 0) {
    //     perror("sendto");
    //     close(sock);
    //     return 1;
    // }

    // // std::cout << "Packet sent: "
    // //           << sent
    // //           << " bytes\n";

    // std::cout << "Packet sent\n";
    // std::cout << "  connection: " << packet.connection_id << '\n';
    // std::cout << "  sequence:   " << packet.sequence << '\n';

    // 4. Receive server response
    char buffer[1024];

    sockaddr_in serverResponse{};
    socklen_t serverLen = sizeof(serverResponse);

    ssize_t bytes = recvfrom(
        sock,
        buffer,
        sizeof(buffer) - 1,
        0,
        reinterpret_cast<sockaddr*>(&serverResponse),
        &serverLen
    );

    if (bytes < 0) {
        perror("recvfrom");
    }
    else {
        buffer[bytes] = '\0';

        std::cout << "Server replied: "
                  << buffer
                  << '\n';
    }

    close(sock);
}