#pragma once

#include <mutex>
#include <set>
#include <map>
#include "udpsocket.h"
#include "message.h"

class FifoUniformBroadcast {
public:
    explicit FifoUniformBroadcast(UdpSender& sender_, UdpReceiver& receiver, FileWriter& writer, int maj_, int self_id_,
                                  int n_)
            : sender{sender_},
              receiver{receiver},
              fileWriter{writer},
              maj{maj_}, self_id{self_id_}, n{n_},
              delivered(static_cast<unsigned long>(n + 1)),
              messages(static_cast<unsigned long>(n + 1)),
              max_delivered(static_cast<unsigned long>(n + 1), -1) {
        receiver.onMessage([this](const Message& message) {
            if (message.getType() != Message::Type::Simple) {
                return;
            }
            int from = message.getData()[0];
            int mes_seq = message.getData()[1];
            bool need_to_send = false;

            {
                std::lock_guard<std::mutex> lock(mutex);
                if (mes_seq <= max_delivered[static_cast<std::size_t>(from)]) {
                    return;
                }
                if (delivered[static_cast<std::size_t>(from)][mes_seq] == 0) {
                    need_to_send = true;
                    messages[static_cast<std::size_t>(from)][mes_seq] = message.getData();
                }
                delivered[static_cast<std::size_t>(from)][mes_seq]++;
//                if ((from == self_id && delivered[static_cast<std::size_t>(from)][mes_seq] == maj)
//                    || (from != self_id && delivered[static_cast<std::size_t>(from)][mes_seq] == maj - 1)) {
//                    fileWriter << "d " + std::to_string(from) + " " +
//                                  std::to_string(message.getData()[2]);
//                }
                if (mes_seq == max_delivered[static_cast<std::size_t>(from)] + 1) {
                    while ((from == self_id && delivered[static_cast<std::size_t>(from)][mes_seq] >= maj)
                           || (from != self_id && delivered[static_cast<std::size_t>(from)][mes_seq] >= maj - 1)) {
                        fileWriter << "d " + std::to_string(from) + " " +
                                        std::to_string(messages[static_cast<std::size_t>(from)][mes_seq][2]);
                        max_delivered[static_cast<std::size_t>(from)]++;
                        messages[static_cast<std::size_t>(from)].erase(mes_seq);
                        delivered[static_cast<std::size_t>(from)].erase(mes_seq);
                        mes_seq++;
                    }
                }
            }
            if (need_to_send) {
                for (int i = 1; i <= n; i++) {
                    if (i == self_id) {
                        continue;
                    }
                    sender.send(Message::simple(self_id, i, message.getData()));
                }
            }
        });
    }

    void broadcast(const std::vector<int>& data) {
        std::vector<int> message_data;
        message_data.push_back(self_id);
        int cur_seq = seq++;
        message_data.push_back(cur_seq);
        message_data.insert(message_data.end(), data.begin(), data.end());
        {
            std::lock_guard<std::mutex> lock(mutex);
            delivered[static_cast<std::size_t>(self_id)][cur_seq]++;
            messages[static_cast<std::size_t>(self_id)][cur_seq] = message_data;
        }
        for (int i = 1; i <= n; i++) {
            if (i == self_id) {
                continue;
            }
            sender.send(Message::simple(self_id, i, message_data));
        }
    }

private:
    UdpSender& sender;
    UdpReceiver& receiver;
    FileWriter& fileWriter;
    int maj, self_id, n;
    std::atomic<int> seq = 0;

    std::mutex mutex;
    std::vector<std::map<int, int>> delivered;
    std::vector<std::map<int, std::vector<int>>> messages;
    std::vector<int> max_delivered;
};