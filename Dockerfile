FROM debian:bookworm

RUN apt-get update && \
    apt-get install -y g++ && \
    rm -rf /var/lib/apt/lists/*

COPY packet.hpp /packet.hpp
COPY server/server.cpp /server.cpp


# RUN g++ -std=c++23 -O2 /server.cpp -o /server
RUN g++ -std=c++23 -O2 /server.cpp -I/ -o /server

EXPOSE 5000/udp

CMD ["/server"]

