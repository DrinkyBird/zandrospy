#include <cstring>
#include <sstream>
#include "buffer.h"
#include "huffman/huffman.h"

Buffer::Buffer(uint8_t *data, size_t length) :
    data(data), length(length), position(0), shouldDelete(false) {}

Buffer::Buffer(size_t length) :
    data(new uint8_t[length]), length(length), position(0), shouldDelete(true) {
    memset(data, 0, length);
}

Buffer::~Buffer() {
    if (shouldDelete) {
        delete[] data;
    }
}


size_t Buffer::tell() {
    return position;
}

void Buffer::seek(size_t pos) {
    if (pos < 0) {
        position = 0;
    } else if (pos > length) {
        position = length;
    } else {
        position = pos;
    }
}

void Buffer::advance(size_t by) {
    seek(tell() + by);
}

std::string Buffer::readString() {
    size_t start = position;
    size_t sl = 0;

    while (position < length) {
        if (data[start + sl] == 0) {
            break;
        }

        sl++;
    }

    std::stringstream ss;

    for (size_t i = 0; i < sl; i++) {
        ss << read<char>();
    }

    position++;
    return ss.str();
}

void Buffer::huffmanify() {
    const int insize = (int)length;
    int outsize = (int)length * 2;
    auto *work = new uint8_t[outsize];
    memset(work, 0, outsize);
    HUFFMAN_Encode(data, work, insize, &outsize);

    delete[] data;
    data = work;
    length = (size_t)outsize;
}

void Buffer::dehuffmanify() {
    const int insize = (int)length;
    int outsize = (int)length * 2;
    auto *work = new uint8_t[outsize];
    memset(work, 0, outsize);
    HUFFMAN_Decode(data, work, insize, &outsize);

    delete[] data;
    data = work;
    length = (size_t)outsize;
}

uint8_t *Buffer::getData() const {
    return this->data;
}

size_t Buffer::getLength() const {
    return this->length;
}

void Buffer::save(const std::string &path) {
    FILE *f = fopen(path.c_str(), "wb");
    if (f != nullptr) {
        fwrite(data, length, sizeof(uint8_t), f);
        fclose(f);
    }
}

void Buffer::resize(size_t newSize) {
    auto *newData = new uint8_t[newSize];
    memset(newData, 0, newSize);
    memcpy(newData, data, std::min(newSize, length));

    delete[] data;
    data = newData;
    length = newSize;
}