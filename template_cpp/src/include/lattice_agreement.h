#pragma once

#include <cassert>
#include <memory>
#include <condition_variable>

enum class MessageType {
    Proposal,
    Ack,
    Nack,
};

class Semaphore {
public:
    explicit Semaphore(int initial_count = 0) : count(initial_count) {
        if (initial_count < 0) {
            throw std::invalid_argument("Semaphore initial count cannot be negative.");
        }
    }

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return count > 0; });
        --count;
    }

    void release() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            ++count;
        }
        cv.notify_one();
    }

//    bool try_acquire() {
//        std::lock_guard<std::mutex> lock(mtx);
//        if (count > 0) {
//            --count;
//            return true;
//        }
//        return false;
//    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};


class Feedback {
public:
    explicit Feedback(FileWriter& writer, Semaphore& sem) : fileWriter{writer}, sem{sem} {}

    void call(int index, const std::string& data) {
//        fileWriter << data;
        {
            std::lock_guard lock(mu);
            datas[index] = data;
            if (index == cur_index) {
//                fileWriter << datas[index];
//                datas.erase(index);
//                cur_index++;
                while (datas.find(cur_index) != datas.end()) {
                    fileWriter << datas[cur_index];
                    std::cout << "final " << cur_index << " " << datas[cur_index] << std::endl;
                    datas.erase(cur_index);
                    cur_index++;
                }
            }
        }

        sem.release();
    }

private:
    FileWriter& fileWriter;
    Semaphore& sem;
    std::mutex mu{};
    int cur_index = 0;
    std::map<int, std::string> datas;
};

class LatticeAgreement {

public:
    LatticeAgreement(int index_, UdpSender& sender_, UdpReceiver& receiver_, Feedback& feedback_, int maj_, int self_id_,
                     int n_)
            : index{index_},
              sender{sender_},
              receiver{receiver_},
              feedback{feedback_},
              maj{maj_}, self_id{self_id_}, n{n_} {
//        receiver.onMessage([this](const Message& message) {
//
//            if (message.getType() != Message::Type::Simple) {
//                return;
//            }
//            int proposal_number;
//            std::set<int> proposed_value_;
//
//            // output message.getData() to see the content of the message
////            std::cout << "Message data: ";
////            for (int i = 0; i < static_cast<int>(message.getData().size()); i++) {
////                std::cout << message.getData()[static_cast<unsigned long>(i)] << " ";
////            }
//            assert(message.getData()[0] == index);
//
//            switch (static_cast<MessageType>(message.getData()[1])) {
//                case MessageType::Proposal:
//                    proposal_number = message.getData()[2];
//                    for (int i = 3; i < static_cast<int>(message.getData().size()); i++) {
//                        proposed_value_.insert(message.getData()[static_cast<unsigned long>(i)]);
//                    }
//                    recv_proposal(message.getFrom(), proposal_number, proposed_value_);
//                    break;
//                case MessageType::Ack:
//                    recv_ack(message.getData()[2]);
//                    break;
//                case MessageType::Nack:
//                    proposal_number = message.getData()[2];
//                    for (int i = 3; i < static_cast<int>(message.getData().size()); i++) {
//                        proposed_value_.insert(message.getData()[static_cast<unsigned long>(i)]);
//                    }
//                    recv_nack(proposal_number, proposed_value_);
//                    break;
//                default:
//                    throw std::runtime_error("Unknown message type");
//            }
//        });

    }

    void propose(const std::set<int>& proposal) {
        std::lock_guard lock(*mutex);
        proposed_value = proposal;
        active = true;
        active_proposal_number++;
        ack_count = 0;
        nack_count = 0;
        recv_nack_ack_delayed();
        broadcast_proposal();
    }

public:
    void broadcast_proposal() {
        std::lock_guard lock(*mutex);
        for (int i = 1; i <= n; i++) {
            std::vector<int> data;
            data.push_back(index);
            data.push_back(static_cast<const int>(MessageType::Proposal));
            data.push_back(active_proposal_number);
            for (int value: proposed_value) {
                data.push_back(value);
            }
            if (i == self_id) {
                recv_proposal(self_id, active_proposal_number, proposed_value);
            } else {
            sender.send(Message::simple(self_id, i, data));
            }
        }
    }

    void recv_ack(int proposal_number) {
        std::lock_guard lock(*mutex);
        if (proposal_number == active_proposal_number) {
            ack_count++;
            nack_ack_check();
        } else if (active_proposal_number < proposal_number) {
            acks[proposal_number]++;
        }
    }

    void recv_nack_ack_delayed() {
        std::lock_guard lock(*mutex);
        ack_count += acks[active_proposal_number];
        nack_count += nacks[active_proposal_number].first;
        for (int v: nacks[active_proposal_number].second) {
            proposed_value.insert(v);
        }
        nack_ack_check();
    }

    void recv_nack(int proposal_number, const std::set<int>& value) {
        std::lock_guard lock(*mutex);

        if (proposal_number == active_proposal_number) {
            nack_count++;
            for (int v: value) {
                proposed_value.insert(v);
            }
            nack_ack_check();
        } else if (active_proposal_number < proposal_number) {
//            nacks[proposal_number] = value;
            // insert value to nacks[proposal_number]
            for (int v: value) {
                nacks[proposal_number].second.insert(v);
            }
            nacks[proposal_number].first++;
        }
    }

    void recv_proposal(int from, int proposal_number, const std::set<int>& proposed_value_) {
        std::lock_guard lock(*mutex);
        bool subset = true;
        for (int value: accepted_value) {
            if (proposed_value_.find(value) == proposed_value_.end()) {
                subset = false;
                break;
            }
        }
//        std::cout << "Epoch: " << active_proposal_number << "\n";
//        std::cout << "Proposed value: ";
//        for (int value: proposed_value_) {
//            std::cout << value << " ";
//        }
//        std::cout << "\n";
//        std::cout << "Accepted value: ";
//        for (int value: accepted_value) {
//            std::cout << value << " ";
//        }
//        std::cout << "\n";
//        std::cout << "Subset? " << subset << "\n";
//        std::cout << "\n" << std::endl;
        if (subset) {
            accepted_value = proposed_value_;
            if (from == self_id) {
                recv_ack(proposal_number);
            } else {
            sender.send(Message::simple(self_id, from, {index, static_cast<int>(MessageType::Ack), proposal_number}));
            }
        } else {
            for (int value: proposed_value_) {
                accepted_value.insert(value);
            }
            std::vector<int> data;
            data.push_back(index);
            data.push_back(static_cast<int>(MessageType::Nack));
            data.push_back(proposal_number);
            for (int value: accepted_value) {
                data.push_back(value);
            }
            if (from == self_id) {
                recv_nack(proposal_number, accepted_value);
            } else {
            sender.send(Message::simple(self_id, from, data));
            }
        }
    }

    void nack_ack_check() {
        std::lock_guard lock(*mutex);
        if (nack_count > 0 && ack_count + nack_count >= maj && active) {
            nacks.erase(active_proposal_number);
            active_proposal_number++;
            ack_count = 0;
            nack_count = 0;
            recv_nack_ack_delayed();
            broadcast_proposal();
        } else if (ack_count >= maj && active) {
            active = false;
            std::string data;
            for (int value: proposed_value) {
                data += std::to_string(value) + " ";
            }
            std::cout << index << " " << data << std::endl;
//            fileWriter << data;
            feedback.call(index, data);
        }
    }

    int index;
    UdpSender& sender;
    UdpReceiver& receiver;
    Feedback& feedback;
    int maj;
    int self_id;
    int n;

    bool active = false;
    int ack_count = 0;
    int nack_count = 0;
    int active_proposal_number = 0;
    std::set<int> proposed_value = {};
    std::set<int> accepted_value = {};
    std::map<int, int> acks;
    std::map<int, std::pair<int, std::set<int>>> nacks;

    std::shared_ptr<std::recursive_mutex> mutex = std::make_shared<std::recursive_mutex>();
//    std::recursive_mutex mutex;


};