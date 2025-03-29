#pragma once

#include "bencode_parser.hpp"
#include <string>
#include <vector>
#include <memory>

struct FileInfo {
    std::string path;
    size_t length;
    size_t offset;
};

struct TorrentInfo {
    std::string name;
    size_t piece_length;
    std::string pieces;  // 20-byte SHA1 hashes concatenated
    std::vector<FileInfo> files;
    size_t total_length;
    std::string info_hash;  // SHA1 hash of the info dictionary
};

class TorrentFile {
public:
    explicit TorrentFile(const std::string& filename);
    
    // Getters for torrent metadata
    const std::vector<std::string>& getAnnounceUrls() const { return announce_urls_; }
    const TorrentInfo& getInfo() const { return info_; }
    const std::string& getComment() const { return comment_; }
    const std::string& getCreatedBy() const { return created_by_; }
    time_t getCreationDate() const { return creation_date_; }
    
    // Utility methods
    std::string getInfoHash() const;
    size_t getNumPieces() const;
    std::string getPieceHash(size_t index) const;
    
private:
    void parseAnnounceUrls(const bencode::BencodeDict& dict);
    void parseInfo(const bencode::BencodeDict& dict);
    void parseFiles(const bencode::BencodeDict& info_dict);
    std::string calculateInfoHash(const bencode::BencodeDict& info_dict);
    
    std::vector<std::string> announce_urls_;
    TorrentInfo info_;
    std::string comment_;
    std::string created_by_;
    time_t creation_date_;
}; 