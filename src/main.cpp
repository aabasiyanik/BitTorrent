#include "torrent_file.hpp"
#include "tracker_client.hpp"
#include "logger.hpp"
#include <iostream>
#include <iomanip>

void printTorrentInfo(const TorrentFile& torrent) {
    std::cout << "Torrent Information:" << std::endl;
    std::cout << "Name: " << torrent.getInfo().name << std::endl;
    std::cout << "Piece Length: " << torrent.getInfo().piece_length << " bytes" << std::endl;
    std::cout << "Total Length: " << torrent.getInfo().total_length << " bytes" << std::endl;
    std::cout << "Number of Pieces: " << torrent.getNumPieces() << std::endl;
    std::cout << "Info Hash: " << torrent.getInfoHash() << std::endl;
    
    std::cout << "\nFiles:" << std::endl;
    for (const auto& file : torrent.getInfo().files) {
        std::cout << "- " << file.path << " (" << file.length << " bytes)" << std::endl;
    }
    
    std::cout << "\nAnnounce URLs:" << std::endl;
    for (const auto& url : torrent.getAnnounceUrls()) {
        std::cout << "- " << url << std::endl;
    }
}

void printTrackerResponse(const TrackerResponse& response) {
    std::cout << "\nTracker Response:" << std::endl;
    if (!response.failure_reason.empty()) {
        std::cout << "Error: " << response.failure_reason << std::endl;
        return;
    }
    
    if (!response.warning_message.empty()) {
        std::cout << "Warning: " << response.warning_message << std::endl;
    }
    
    std::cout << "Interval: " << response.interval << " seconds" << std::endl;
    if (response.min_interval > 0) {
        std::cout << "Min Interval: " << response.min_interval << " seconds" << std::endl;
    }
    std::cout << "Complete (Seeders): " << response.complete << std::endl;
    std::cout << "Incomplete (Leechers): " << response.incomplete << std::endl;
    
    std::cout << "\nPeers (" << response.peers.size() << "):" << std::endl;
    for (const auto& peer : response.peers) {
        std::cout << "- " << peer.ip << ":" << peer.port << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <torrent_file>" << std::endl;
        return 1;
    }
    
    try {
        // Parse torrent file
        TorrentFile torrent(argv[1]);
        printTorrentInfo(torrent);
        
        // Create tracker client
        TrackerClient tracker;
        
        // Generate a random peer ID (in a real client, this would be more sophisticated)
        std::string peer_id = "-BT0001-";
        for (int i = 0; i < 12; ++i) {
            peer_id += static_cast<char>('0' + (rand() % 10));
        }
        
        // Announce to tracker
        TrackerResponse response = tracker.announce(
            torrent.getAnnounceUrls()[0],  // Use first tracker
            torrent.getInfoHash(),
            peer_id,
            6881,  // Default BitTorrent port
            0,     // Uploaded bytes
            0,     // Downloaded bytes
            torrent.getInfo().total_length,  // Left to download
            true,  // Use compact format
            false, // Include peer ID
            true,  // Include event
            "started"  // Initial event
        );
        
        printTrackerResponse(response);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 