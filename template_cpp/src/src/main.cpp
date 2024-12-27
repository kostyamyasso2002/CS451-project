#include <chrono>
#include <iostream>
#include <thread>

#include "parser.hpp"
#include <csignal>
#include "receiver.h"
#include "sender.h"
#include "file_writer.h"
#include "message.h"
#include "fifo_uniform_broadcast.h"

static void stop(int) {
    // reset signal handlers to default
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    // immediately stop network packet processing
    std::cout << "Immediately stopping network packet processing.\n";

    // write/flush output file if necessary
    std::cout << "Writing output.\n";

    std::cout.flush();

    // exit directly from signal handler
    exit(0);
}


Host findHostById(const std::vector<Host>& hosts, unsigned long id) {
    auto it = std::find_if(hosts.begin(), hosts.end(), [id](const Host& host) {
        return host.id == id;
    });

    if (it == hosts.end()) {
        throw std::runtime_error("Host with id " + std::to_string(id) + " not found");
    }

    return *it;
}

int main(int argc, char** argv) {
    signal(SIGTERM, stop);
    signal(SIGINT, stop);

    // `true` means that a config file is required.
    // Call with `false` if no config file is necessary.
    bool requireConfig = true;

    Parser parser(argc, argv);
    parser.parse();

    std::cout << std::endl;

    std::cout << "My PID: " << getpid() << "\n";
    std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
              << getpid() << "` to stop processing packets\n\n";

    std::cout << "My ID: " << parser.id() << "\n\n";

    std::cout << "List of resolved hosts is:\n";
    std::cout << "==========================\n";
    auto hosts = parser.hosts();
    for (auto& host: hosts) {
        std::cout << host.id << "\n";
        std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
        std::cout << "Machine-readable IP: " << host.ip << "\n";
        std::cout << "Human-readable Port: " << host.portReadable() << "\n";
        std::cout << "Machine-readable Port: " << host.port << "\n";
        std::cout << "\n";
    }
    std::cout << "\n";

    std::cout << "Path to output:\n";
    std::cout << "===============\n";
    std::cout << parser.outputPath() << "\n\n";

    std::cout << "Path to config:\n";
    std::cout << "===============\n";
    std::cout << parser.configPath() << "\n\n";

    std::cout << "Doing some initialization...\n\n";

    UdpSocket socket{parser.id(), findHostById(hosts, parser.id()).port};
    UdpSender sender{socket, hosts};
    UdpReceiver receiver{socket, sender};
    FileWriter fileWriter{parser.outputPath()};

    // read config file
    int messagesNum;
    uint32_t receiverId;
    if (requireConfig) {
        std::ifstream configFile(parser.configPath());
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open config file");
        }


        configFile >> messagesNum
        >> receiverId
        ;
    } else {
        messagesNum = 10;
        receiverId = 2;
    }

    std::cout << "Broadcasting and delivering messages...\n\n";

    // Create a UDP socket for broadcasting messages.

//    receiver.onMessage([&fileWriter](const Message& message) {
//        if (message.getType() == Message::Type::Simple) {
//            std::string data = "d ";
//            data += std::to_string(message.getFrom()) + " " + std::to_string(message.getData()[0]);
//
//            fileWriter << data;
//        }
//    });
//
//    if (parser.id() != receiverId) {
//        for (int i = 1; i <= messagesNum; i++) {
//            sender.send(Message::simple(static_cast<int>(parser.id()), static_cast<int>(receiverId), {i}));
//            fileWriter << "b " + std::to_string(i);
//        }
//    }

    FifoUniformBroadcast fifoUniformBroadcast{sender, receiver, fileWriter, static_cast<int>(hosts.size() / 2 + 1),
                                              static_cast<int>(parser.id()), static_cast<int>(hosts.size())};


    for (int i = 1; i <= messagesNum; i++) {
        fileWriter << "b " + std::to_string(i);
        fifoUniformBroadcast.broadcast({i});
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }

    return 0;
}
