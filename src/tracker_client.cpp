#include "tracker_client.hpp"
#include "bencode_parser.hpp"
#include "logger.hpp"
#include <sstream>
#include <iomanip>

TrackerClient::TrackerClient() {
    curl_ = curl_easy_init();
    if (!curl_) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

TrackerClient::~TrackerClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

size_t TrackerClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string TrackerClient::urlEncode(const std::string& str) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL for URL encoding");
    }
    
    char* encoded = curl_easy_escape(curl, str.c_str(), str.length());
    if (!encoded) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("Failed to URL encode string");
    }
    
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

std::string TrackerClient::buildAnnounceUrl(const std::string& tracker_url,
                                          const std::string& info_hash,
                                          const std::string& peer_id,
                                          uint64_t port,
                                          uint64_t uploaded,
                                          uint64_t downloaded,
                                          uint64_t left,
                                          bool compact,
                                          bool no_peer_id,
                                          bool event,
                                          const std::string& event_type) {
    std::stringstream ss;
    ss << tracker_url << "?info_hash=" << urlEncode(info_hash)
       << "&peer_id=" << urlEncode(peer_id)
       << "&port=" << port
       << "&uploaded=" << uploaded
       << "&downloaded=" << downloaded
       << "&left=" << left
       << "&compact=" << (compact ? "1" : "0")
       << "&no_peer_id=" << (no_peer_id ? "1" : "0");
       
    if (event) {
        ss << "&event=" << event_type;
    }
    
    return ss.str();
}

TrackerResponse TrackerClient::announce(const std::string& tracker_url,
                                      const std::string& info_hash,
                                      const std::string& peer_id,
                                      uint64_t port,
                                      uint64_t uploaded,
                                      uint64_t downloaded,
                                      uint64_t left,
                                      bool compact,
                                      bool no_peer_id,
                                      bool event,
                                      const std::string& event_type) {
    TrackerResponse response;
    
    std::string url = buildAnnounceUrl(tracker_url, info_hash, peer_id, port,
                                     uploaded, downloaded, left, compact,
                                     no_peer_id, event, event_type);
                                     
    std::string response_data;
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        response.failure_reason = curl_easy_strerror(res);
        return response;
    }
    
    try {
        auto parsed = bencode::BencodeParser::parse(response_data);
        if (!parsed->isDict()) {
            response.failure_reason = "Invalid tracker response: not a dictionary";
            return response;
        }
        
        const auto& dict = parsed->asDict();
        
        // Parse interval
        if (auto interval_it = dict.find("interval"); interval_it != dict.end()) {
            response.interval = interval_it->second->asInteger();
        }
        
        // Parse min interval
        if (auto min_interval_it = dict.find("min interval"); min_interval_it != dict.end()) {
            response.min_interval = min_interval_it->second->asInteger();
        }
        
        // Parse complete/incomplete
        if (auto complete_it = dict.find("complete"); complete_it != dict.end()) {
            response.complete = complete_it->second->asInteger();
        }
        
        if (auto incomplete_it = dict.find("incomplete"); incomplete_it != dict.end()) {
            response.incomplete = incomplete_it->second->asInteger();
        }
        
        // Parse peers
        if (auto peers_it = dict.find("peers"); peers_it != dict.end()) {
            const auto& peers_value = peers_it->second;
            if (peers_value->isString()) {
                response.peers = parseCompactPeers(peers_value->asString());
            } else if (peers_value->isList()) {
                response.peers = parseBencodedPeers(peers_value->encode());
            }
        }
        
        // Parse failure reason
        if (auto failure_it = dict.find("failure reason"); failure_it != dict.end()) {
            response.failure_reason = failure_it->second->asString();
        }
        
        // Parse warning message
        if (auto warning_it = dict.find("warning message"); warning_it != dict.end()) {
            response.warning_message = warning_it->second->asString();
        }
        
    } catch (const std::exception& e) {
        response.failure_reason = std::string("Failed to parse tracker response: ") + e.what();
    }
    
    return response;
}

TrackerResponse TrackerClient::scrape(const std::string& tracker_url,
                                    const std::string& info_hash) {
    TrackerResponse response;
    
    // Convert announce URL to scrape URL
    std::string scrape_url = tracker_url;
    size_t pos = scrape_url.find("announce");
    if (pos != std::string::npos) {
        scrape_url.replace(pos, 9, "scrape");
    }
    
    std::string url = scrape_url + "?info_hash=" + urlEncode(info_hash);
    std::string response_data;
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        response.failure_reason = curl_easy_strerror(res);
        return response;
    }
    
    try {
        auto parsed = bencode::BencodeParser::parse(response_data);
        if (!parsed->isDict()) {
            response.failure_reason = "Invalid tracker response: not a dictionary";
            return response;
        }
        
        const auto& dict = parsed->asDict();
        auto files_it = dict.find("files");
        if (files_it == dict.end()) {
            response.failure_reason = "No files information in scrape response";
            return response;
        }
        
        const auto& files_dict = files_it->second->asDict();
        auto info_it = files_dict.find(info_hash);
        if (info_it == files_dict.end()) {
            response.failure_reason = "No information for requested info_hash";
            return response;
        }
        
        const auto& info_dict = info_it->second->asDict();
        
        // Parse complete/incomplete
        if (auto complete_it = info_dict.find("complete"); complete_it != info_dict.end()) {
            response.complete = complete_it->second->asInteger();
        }
        
        if (auto incomplete_it = info_dict.find("incomplete"); incomplete_it != info_dict.end()) {
            response.incomplete = incomplete_it->second->asInteger();
        }
        
    } catch (const std::exception& e) {
        response.failure_reason = std::string("Failed to parse tracker response: ") + e.what();
    }
    
    return response;
}

std::vector<Peer> TrackerClient::parseCompactPeers(const std::string& peers_str) {
    std::vector<Peer> peers;
    if (peers_str.length() % 6 != 0) {
        throw std::runtime_error("Invalid compact peers string length");
    }
    
    for (size_t i = 0; i < peers_str.length(); i += 6) {
        Peer peer;
        
        // Parse IP address (4 bytes)
        peer.ip = std::to_string(static_cast<unsigned char>(peers_str[i])) + "." +
                 std::to_string(static_cast<unsigned char>(peers_str[i + 1])) + "." +
                 std::to_string(static_cast<unsigned char>(peers_str[i + 2])) + "." +
                 std::to_string(static_cast<unsigned char>(peers_str[i + 3]));
                 
        // Parse port (2 bytes)
        peer.port = (static_cast<unsigned char>(peers_str[i + 4]) << 8) |
                    static_cast<unsigned char>(peers_str[i + 5]);
                    
        peers.push_back(peer);
    }
    
    return peers;
}

std::vector<Peer> TrackerClient::parseBencodedPeers(const std::string& peers_str) {
    std::vector<Peer> peers;
    auto parsed = bencode::BencodeParser::parse(peers_str);
    
    if (!parsed->isList()) {
        throw std::runtime_error("Invalid peers list format");
    }
    
    const auto& peers_list = parsed->asList();
    for (const auto& peer_value : peers_list) {
        if (!peer_value->isDict()) {
            throw std::runtime_error("Invalid peer format");
        }
        
        const auto& peer_dict = peer_value->asDict();
        Peer peer;
        
        if (auto ip_it = peer_dict.find("ip"); ip_it != peer_dict.end()) {
            peer.ip = ip_it->second->asString();
        }
        
        if (auto port_it = peer_dict.find("port"); port_it != peer_dict.end()) {
            peer.port = port_it->second->asInteger();
        }
        
        peers.push_back(peer);
    }
    
    return peers;
} 