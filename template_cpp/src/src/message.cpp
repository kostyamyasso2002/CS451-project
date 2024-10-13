#include <vector>
#include "message.h"

Message Message::simple(int from, int to, const std::vector<int>& data) {
    return {Type::Simple, from, to, counter++, data};
}

Message Message::ack(int from, int to, int id) {
    return {Type::Ack, from, to, counter++, {id}};
}

std::atomic<int> Message::counter = 0;
