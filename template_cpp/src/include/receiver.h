#pragma once

#include "udpsocket.h"
#include "sender.h"

class UdpReceiver {
public:
    explicit UdpReceiver(const UdpSocket& socket, UdpSender& sender) : socket{socket}, sender{sender} {
//        callbacks.emplace_back([this](const Message& message) {
//            if (message.getType() == Message::Type::Simple) {
//                this->sender.send(
//                        Message::ack(message.getTo(), message.getFrom(), message.getId()), message.getFrom()
//                        );
//            }
//        });
    }

    void receive() {
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        char buffer[1024];

        ssize_t n = recvfrom(socket.fd(), buffer, sizeof(buffer), MSG_WAITALL,
                             reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);

        if (n < 0) {
            // print error in nice way
            std::cout << "Failed to receive message, error " << n << std::endl;
            throw std::runtime_error("Failed to receive message");
        }

        if (n % 4 != 0) {
            throw std::runtime_error("Invalid message length");
        }

        // convert buffer to vector of int
        std::vector<int> data;
        data.reserve(static_cast<unsigned long>(n / 4));
        for (int i = 0; i < n / 4; i++) {
            data.push_back(*reinterpret_cast<int*>(buffer + i * 4));
        }

//        for (auto i : data) {
//            std::cout << i << " ";
//        }
//        std::cout << std::endl;

        auto message = Message::fromRawData(data);

        if (message.getType() == Message::Type::Ack) {
            sender.messageReceived(message.getData()[0]);
        }
        if (message.getType() == Message::Type::Simple) {
            sender.send(
                    Message::ack(message.getTo(), message.getFrom(), message.getId()), message.getFrom()
                    );
        }

        if (received_ids.count({message.getFrom(), message.getId()}) == 0) {
            received_ids.insert({message.getFrom(), message.getId()});

            for (const auto& callback: callbacks) {
                callback(message);
            }
        }

//        std::lock_guard<std::mutex> lock(mutex);
//        for (const auto& callback: callbacks) {
//            callback(message);
//        }
    }

    void onMessage(std::function<void(const Message&)> callback) {
        std::lock_guard<std::mutex> lock(mutex);
        callbacks.push_back(std::move(callback));
    }

private:
    const UdpSocket& socket;
    UdpSender& sender;

    std::mutex mutex;
    std::vector<std::function<void(const Message&)>> callbacks;
    std::set<std::pair<int, int>> received_ids;
};