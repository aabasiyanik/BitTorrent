cmake_minimum_required(VERSION 3.10)
project(BitTorrent VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/bencode_parser.cpp
    src/torrent_file.cpp
    src/tracker_client.cpp
)

# Add header files
set(HEADERS
    include/bencode_parser.hpp
    include/torrent_file.hpp
    include/tracker_client.hpp
    include/logger.hpp
)

# Create executable
add_executable(bittorrent ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(bittorrent PRIVATE include)

# Link libraries
target_link_libraries(bittorrent PRIVATE
    CURL::libcurl
    Boost::system
    OpenSSL::Crypto
) 