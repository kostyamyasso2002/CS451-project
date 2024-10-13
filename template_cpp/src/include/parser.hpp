#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <algorithm>
#include <cctype>
#include <locale>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "host.h"

class Parser {
public:
    Parser(const int argc, char const *const *argv, bool withConfig = true)
            : argc{argc}, argv{argv}, withConfig{withConfig}, parsed{false} {}

    void parse() {
        if (!parseInternal()) {
            help(argc, argv);
        }

        parsed = true;
    }

    [[nodiscard]] unsigned long id() const {
        checkParsed();
        return id_;
    }

    [[nodiscard]] const char *hostsPath() const {
        checkParsed();
        return hostsPath_.c_str();
    }

    [[nodiscard]] const char *outputPath() const {
        checkParsed();
        return outputPath_.c_str();
    }

    [[nodiscard]] const char *configPath() const {
        checkParsed();
        if (!withConfig) {
            throw std::runtime_error("Parser is configure to ignore the config path");
        }

        return configPath_.c_str();
    }

    [[nodiscard]] std::vector<Host> hosts() const {
        std::ifstream hostsFile(hostsPath());
        std::vector<Host> hosts;

        if (!hostsFile.is_open()) {
            std::ostringstream os;
            os << "`" << hostsPath() << "` does not exist.";
            throw std::invalid_argument(os.str());
        }

        std::string line;
        int lineNum = 0;
        while (std::getline(hostsFile, line)) {
            lineNum += 1;

            std::istringstream iss(line);

            trim(line);
            if (line.empty()) {
                continue;
            }

            unsigned long id;
            std::string ip;
            unsigned short port;

            if (!(iss >> id >> ip >> port)) {
                std::ostringstream os;
                os << "Parsing for `" << hostsPath() << "` failed at line " << lineNum;
                throw std::invalid_argument(os.str());
            }

            hosts.emplace_back(id, ip, port);
        }

        if (hosts.size() < 2UL) {
            std::ostringstream os;
            os << "`" << hostsPath() << "` must contain at least two hosts";
            throw std::invalid_argument(os.str());
        }

        auto comp = [](const Host &x, const Host &y) { return x.id < y.id; };
        auto result = std::minmax_element(hosts.begin(), hosts.end(), comp);
        size_t minID = (*result.first).id;
        size_t maxID = (*result.second).id;
        if (minID != 1UL || maxID != static_cast<unsigned long>(hosts.size())) {
            std::ostringstream os;
            os << "In `" << hostsPath()
               << "` IDs of processes have to start from 1 and be compact";
            throw std::invalid_argument(os.str());
        }

        std::sort(hosts.begin(), hosts.end(),
                  [](const Host &a, const Host &b) -> bool { return a.id < b.id; });

        return hosts;
    }

private:
    bool parseInternal() {
        if (!parseID()) {
            return false;
        }

        if (!parseHostPath()) {
            return false;
        }

        if (!parseOutputPath()) {
            return false;
        }

        if (!parseConfigPath()) {
            return false;
        }

        return true;
    }

    void help(const int, char const *const *argv_) const {
        auto configStr = "CONFIG";
        std::cerr << "Usage: " << argv_[0]
                  << " --id ID --hosts HOSTS --output OUTPUT";

        if (!withConfig) {
            std::cerr << "\n";
        } else {
            std::cerr << " CONFIG\n";
        }

        exit(EXIT_FAILURE);
    }

    bool parseID() {
        if (argc < 3) {
            return false;
        }

        if (std::strcmp(argv[1], "--id") == 0) {
            if (isPositiveNumber(argv[2])) {
                try {
                    id_ = std::stoul(argv[2]);
                } catch (std::invalid_argument const &e) {
                    return false;
                } catch (std::out_of_range const &e) {
                    return false;
                }

                return true;
            }
        }

        return false;
    }

    bool parseHostPath() {
        if (argc < 5) {
            return false;
        }

        if (std::strcmp(argv[3], "--hosts") == 0) {
            hostsPath_ = std::string(argv[4]);
            return true;
        }

        return false;
    }

    bool parseOutputPath() {
        if (argc < 7) {
            return false;
        }

        if (std::strcmp(argv[5], "--output") == 0) {
            outputPath_ = std::string(argv[6]);
            return true;
        }

        return false;
    }

    bool parseConfigPath() {
        if (!withConfig) {
            return true;
        }

        if (argc < 8) {
            return false;
        }

        configPath_ = std::string(argv[7]);
        return true;
    }

    static bool isPositiveNumber(const std::string &s) {
        return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
            return !std::isdigit(c);
        }) == s.end();
    }

    void checkParsed() const {
        if (!parsed) {
            throw std::runtime_error("Invoke parse() first");
        }
    }

    static void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                        [](int ch) { return !std::isspace(ch); }));
    }

    static void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                             [](int ch) { return !std::isspace(ch); })
                        .base(),
                s.end());
    }

    static void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

private:
    const int argc;
    char const *const *argv;
    bool withConfig;

    bool parsed;

    unsigned long id_{};
    std::string hostsPath_;
    std::string outputPath_;
    std::string configPath_;
};
