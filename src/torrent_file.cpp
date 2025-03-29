#include "torrent_file.hpp"
#include "logger.hpp"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
// test
TorrentFile::TorrentFile(const std::string& filename) {
    auto parsed = bencode::BencodeParser::parseFile(filename);
    if (!parsed->isDict()) {
        throw std::runtime_error("Invalid torrent file: root must be a dictionary");
    }
    
    const auto& dict = parsed->asDict();
    
    // Parse announce URLs
    parseAnnounceUrls(dict);
    
    // Parse info dictionary
    auto it = dict.find("info");
    if (it == dict.end()) {
        throw std::runtime_error("Missing info dictionary in torrent file");
    }
    parseInfo(it->second->asDict());
    
    // Parse optional fields
    if (auto comment_it = dict.find("comment"); comment_it != dict.end()) {
        comment_ = comment_it->second->asString();
    }
    
    if (auto created_by_it = dict.find("created by"); created_by_it != dict.end()) {
        created_by_ = created_by_it->second->asString();
    }
    
    if (auto creation_date_it = dict.find("creation date"); creation_date_it != dict.end()) {
        creation_date_ = creation_date_it->second->asInteger();
    }
}

void TorrentFile::parseAnnounceUrls(const bencode::BencodeDict& dict) {
    // Handle both single announce URL and announce-list
    if (auto announce_it = dict.find("announce"); announce_it != dict.end()) {
        announce_urls_.push_back(announce_it->second->asString());
    }
    
    if (auto announce_list_it = dict.find("announce-list"); announce_list_it != dict.end()) {
        const auto& announce_list = announce_list_it->second->asList();
        for (const auto& tier : announce_list) {
            if (tier->isList()) {
                for (const auto& url : tier->asList()) {
                    announce_urls_.push_back(url->asString());
                }
            }
        }
    }
}

void TorrentFile::parseInfo(const bencode::BencodeDict& info_dict) {
    // Parse name
    if (auto name_it = info_dict.find("name"); name_it != info_dict.end()) {
        info_.name = name_it->second->asString();
    }
    
    // Parse piece length
    if (auto piece_length_it = info_dict.find("piece length"); piece_length_it != info_dict.end()) {
        info_.piece_length = piece_length_it->second->asInteger();
    }
    
    // Parse pieces
    if (auto pieces_it = info_dict.find("pieces"); pieces_it != info_dict.end()) {
        info_.pieces = pieces_it->second->asString();
    }
    
    // Parse files
    parseFiles(info_dict);
    
    // Calculate info hash
    info_.info_hash = calculateInfoHash(info_dict);
}

void TorrentFile::parseFiles(const bencode::BencodeDict& info_dict) {
    info_.total_length = 0;
    
    // Handle both single file and multiple files
    if (auto files_it = info_dict.find("files"); files_it != info_dict.end()) {
        // Multiple files
        const auto& files = files_it->second->asList();
        size_t offset = 0;
        
        for (const auto& file : files) {
            const auto& file_dict = file->asDict();
            FileInfo file_info;
            
            // Parse file path
            if (auto path_it = file_dict.find("path"); path_it != file_dict.end()) {
                const auto& path_list = path_it->second->asList();
                std::stringstream path_ss;
                for (size_t i = 0; i < path_list.size(); ++i) {
                    if (i > 0) path_ss << "/";
                    path_ss << path_list[i]->asString();
                }
                file_info.path = path_ss.str();
            }
            
            // Parse file length
            if (auto length_it = file_dict.find("length"); length_it != file_dict.end()) {
                file_info.length = length_it->second->asInteger();
            }
            
            file_info.offset = offset;
            offset += file_info.length;
            
            info_.files.push_back(file_info);
            info_.total_length += file_info.length;
        }
    } else if (auto length_it = info_dict.find("length"); length_it != info_dict.end()) {
        // Single file
        FileInfo file_info;
        file_info.path = info_.name;
        file_info.length = length_it->second->asInteger();
        file_info.offset = 0;
        
        info_.files.push_back(file_info);
        info_.total_length = file_info.length;
    }
}

std::string TorrentFile::calculateInfoHash(const bencode::BencodeDict& info_dict) {
    bencode::BencodeValue value(info_dict);
    std::string info_str = value.encode();
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(info_str.c_str()), info_str.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string TorrentFile::getInfoHash() const {
    return info_.info_hash;
}

size_t TorrentFile::getNumPieces() const {
    return info_.pieces.length() / 20;  // Each piece hash is 20 bytes
}

std::string TorrentFile::getPieceHash(size_t index) const {
    if (index >= getNumPieces()) {
        throw std::out_of_range("Piece index out of range");
    }
    return info_.pieces.substr(index * 20, 20);
} 