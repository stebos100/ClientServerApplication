#ifndef MESSAGE_H
#define MESSAGE_H

#include <array>

struct message_t {
    std::array<char, 64> symbol;
    double net_position;
    std::array<char, 32> timestamp;

    message_t() {
        
        timestamp.fill(0);
        net_position = 0.0;
        timestamp.fill(0);
    }
};

#endif
