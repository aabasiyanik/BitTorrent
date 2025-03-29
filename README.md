# BitTorrent Client

A simple BitTorrent client implementation in C++ that supports:
- Parsing torrent files (bencode format)
- Connecting to trackers
- Displaying torrent metadata
- SHA1 hash calculation for info dictionaries

## Dependencies
- CMake (>= 3.10)
- OpenSSL
- Boost
- libcurl

## Building
```bash
mkdir build
cd build
cmake ..
make
```

## Usage
```bash
./bittorrent <torrent_file>
```

## Features
- Bencode parser for .torrent files
- Support for single and multi-file torrents
- Tracker communication
- Info hash calculation
- Piece verification using SHA1

## Project Structure

- `include/` - Header files
  - `bencode_parser.hpp` - Bencode format parser
  - `torrent_file.hpp` - Torrent file parser
  - `tracker_client.hpp` - Tracker communication

- `src/` - Source files
  - `bencode_parser.cpp` - Bencode parser implementation
  - `torrent_file.cpp` - Torrent file parser implementation
  - `tracker_client.cpp` - Tracker client implementation
  - `main.cpp` - Main program

## License

This project is licensed under the MIT License - see the LICENSE file for details. 