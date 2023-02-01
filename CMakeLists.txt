cmake_minimum_required(VERSION 3.2)
project(zandrospy)

set(CMAKE_CXX_STANDARD 17)

add_executable(zandrospy
    buffer.cpp
    buffer.h
    main.cpp
    main.h
    muninnode.cpp
    muninnode.h
    muninregistration.cpp
    muninregistration.h
    socket.h
    socket.cpp
    util.h
    zanproto.h
    zanquerent.cpp
    zanquerent.h
    zanserver.h

    plugins/testplugin.cpp

    huffman/bitreader.cpp
    huffman/bitreader.h
    huffman/bitwriter.cpp
    huffman/bitwriter.h
    huffman/codec.h
    huffman/huffcodec.cpp
    huffman/huffcodec.h
    huffman/huffman.cpp
    huffman/huffman.h
    plugins/version.cpp)

if (WIN32)
    target_link_libraries(zandrospy PRIVATE ws2_32)
endif ()

target_include_directories(zandrospy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
