#pragma once

#include <cstdint>

struct Packet {
    uint32_t sequence;
    uint32_t connection_id;
    char data[1024];
};