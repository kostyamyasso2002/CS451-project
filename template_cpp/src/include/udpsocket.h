#pragma once

#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>

class UdpSocket {
public:
    explicit UdpSocket(uint64_t proc_id, uint16_t port) : proc_id{proc_id}, port{port} {
        if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);

        if (bind(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }
    }

    ~UdpSocket() {
        if (socket_fd >= 0) {
            close(socket_fd);
        }
    }

    // remove copy constructor
    UdpSocket(const UdpSocket &) = delete;

    UdpSocket &operator=(const UdpSocket &) = delete;

    UdpSocket(UdpSocket &&other) noexcept: proc_id{other.proc_id}, socket_fd{other.socket_fd}, port{other.port} {
        other.socket_fd = -1;
    }

    UdpSocket &operator=(UdpSocket &&other) noexcept {
        if (this != &other) {
            if (socket_fd >= 0) {
                close(socket_fd);
            }
            socket_fd = other.socket_fd;
            port = other.port;
            other.socket_fd = -1;
        }
        return *this;
    }

    [[nodiscard]] int fd() const {
        return socket_fd;
    }

    [[nodiscard]] uint16_t getPort() const {
        return port;
    }

private:
    uint64_t proc_id;
    int socket_fd;
    uint16_t port;
};