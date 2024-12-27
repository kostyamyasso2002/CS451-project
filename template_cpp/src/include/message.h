#include <utility>
#include <atomic>

#pragma once

class Message {
public:
    enum class Type {
        Simple,
        Ack,
    };

    static Message simple(int from, int to, const std::vector<int>& data);

    static Message ack(int from, int to, int id);

    static Message fromRawData(const std::vector<int>& data) {
        return Message{static_cast<Type>(data[0]), data[1], data[2], data[3],
                       std::vector<int>(data.begin() + 4, data.end())};
    }

    [[nodiscard]] Type getType() const {
        return type;
    }

    [[nodiscard]] const std::vector<int>& getData() const {
        return data;
    }

    [[nodiscard]] int getFrom() const {
        return from;
    }

    [[nodiscard]] int getTo() const {
        return to;
    }

    [[nodiscard]] int getId() const {
        return id;
    }

    [[nodiscard]] std::size_t bytes() const {
        return data.size() * sizeof(int);
    }

    [[nodiscard]] const void* dataPtr() const {
        return data.data();
    }

private:
    Message(Type type, int from, int to, int id, std::vector<int> data) : type{type}, from{from}, to{to}, id(id),
                                                                          data{std::move(data)} {}

    Type type;
    int from, to;
    int id;
    std::vector<int> data;

    static std::atomic<int> counter;
};

