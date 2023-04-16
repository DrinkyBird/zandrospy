# zandrospy

[Zandronum](https://zandronum.com) stats as a [Munin](https://munin-monitoring.org) node.

## Building

All you need is a C++17 compiler and CMake.

```
git clone https://github.com/DrinkyBird/zandrospy.git
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j`nproc`
```

## Using

Start the server with `./zandrospy`, or write a systemd service for it or whatever.

Then just add it to your Munin master like any other. The port is hardcoded to `14949`.

```ini
[zandronum]
    address 52.215.132.220
    port 14949
```

## Acknowledgments

Contains Huffman codec code from Zandronum, (c) Timothy Landers and the Zandronum development team.
