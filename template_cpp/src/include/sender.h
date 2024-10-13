#pragma once

#include <mutex>
#include <set>
#include "udpsocket.h"
#include "message.h"

class UdpSender {
public:
    explicit UdpSender(const UdpSocket& socket, std::vector<Host> hosts) : socket{socket}, hosts{std::move(hosts)} {
        // execute thread to send messages
        std::thread([this] {
            while (true) {
                std::vector<std::pair<Message, int>> messages_local;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    for (const auto& to_send: messages) {
                        if (received_ids.count(to_send.first.getId()) == 0) {
                            messages_local.push_back(to_send);
                        }
                    }
                    messages.clear();
                }
                for (const auto& to_send: messages_local) {
                    send(to_send.first, to_send.second);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }).detach();
    }

    void send(const Message& message, int balance = std::numeric_limits<int>::max()) {
        if (balance > 0) {
            std::lock_guard<std::mutex> lock(mutex);
            messages.emplace_back(message, balance - 1);
        }
        struct sockaddr_in dest_addr{};
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = findHostById(hosts, static_cast<unsigned long>(message.getTo())).ip;
        dest_addr.sin_port = htons(findHostById(hosts, static_cast<unsigned long>(message.getTo())).port);

        std::vector<int> rawData{static_cast<int>(message.getType()), message.getFrom(), message.getTo(),
                                 message.getId()};
        rawData.insert(rawData.end(), message.getData().begin(), message.getData().end());

        ssize_t n = sendto(socket.fd(), rawData.data(), rawData.size() * sizeof(int), 0,
                           reinterpret_cast<struct sockaddr*>(&dest_addr), sizeof(dest_addr));

        if (n < 0) {
            throw std::runtime_error("Failed to send message");
        }
    }

    void messageReceived(int id) {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "Received ack " << id << std::endl;
        received_ids.insert(id);
    }

private:
    const UdpSocket& socket;
    const std::vector<Host> hosts;

    std::mutex mutex;
    std::vector<std::pair<Message, int>> messages;
    std::set<int> received_ids;
};