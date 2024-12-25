#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

class Buffer {
public:
    Buffer(uint8_t *data, size_t length);
    explicit Buffer(size_t length);
    ~Buffer();

    [[nodiscard]] size_t tell() const;
    void seek(size_t pos);
    void advance(size_t by);

    template<typename T>
    inline void write(const T value) {
        if (position + sizeof(T) > length) {
            return;
        }

        auto *p = reinterpret_cast<T *>(data + position);
        *p = static_cast<T>(value);
        advance(sizeof(T));
    }

    template<typename T>
    inline T read(const T def = 0) {
        if (position + sizeof(T) > length) {
            return def;
        }

        T value = *(reinterpret_cast<T *>(data + position));
        advance(sizeof(T));
        return value;
    }

    template<typename T>
    inline T peek() const {
        if (position + sizeof(T) >= length) {
            return 0;
        }

        T value = reinterpret_cast<T *>(data + position);
        return value;
    }

    [[nodiscard]] inline bool isEnd() const {
        return position >= length - 1;
    }

    std::string readString();

    void huffmanify();
    void dehuffmanify();

    bool copy(void *src, size_t len, size_t offset);

    [[nodiscard]] uint8_t *getData() const;
    [[nodiscard]] size_t getLength() const;

    void save(const std::string &path);

    void resize(size_t newSize);

private:
    uint8_t *data;
    size_t length, position;
    bool shouldDelete;
};
