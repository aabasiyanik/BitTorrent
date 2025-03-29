#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <curl/curl.h>

struct Peer {
    std::string ip;
    uint16_t port;
};

struct TrackerResponse {
    int64_t interval;          // Time between tracker requests
    int64_t min_interval;      // Minimum time between tracker requests
    int64_t complete;          // Number of seeders
    int64_t incomplete;        // Number of leechers
    std::vector<Peer> peers;   // List of peers
    std::string failure_reason; // Error message if request failed
    std::string warning_message; // Warning message from tracker
};

class TrackerClient {
public:
    TrackerClient();
    ~TrackerClient();
    
    // Main tracker communication methods
    TrackerResponse announce(const std::string& tracker_url,
                           const std::string& info_hash,
                           const std::string& peer_id,
                           uint64_t port,
                           uint64_t uploaded,
                           uint64_t downloaded,
                           uint64_t left,
                           bool compact = true,
                           bool no_peer_id = false,
                           bool event = false,
                           const std::string& event_type = "");
                           
    TrackerResponse scrape(const std::string& tracker_url,
                          const std::string& info_hash);
    
    // Utility methods
    static std::string urlEncode(const std::string& str);
    static std::vector<Peer> parseCompactPeers(const std::string& peers_str);
    static std::vector<Peer> parseBencodedPeers(const std::string& peers_str);
    
private:
    CURL* curl_;
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    static std::string buildAnnounceUrl(const std::string& tracker_url,
                                      const std::string& info_hash,
                                      const std::string& peer_id,
                                      uint64_t port,
                                      uint64_t uploaded,
                                      uint64_t downloaded,
                                      uint64_t left,
                                      bool compact,
                                      bool no_peer_id,
                                      bool event,
                                      const std::string& event_type);
}; 