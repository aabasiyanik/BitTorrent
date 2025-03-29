#include "bencode_parser.hpp"
#include "logger.hpp"
#include <fstream>
#include <stdexcept>

namespace bencode {

std::string BencodeValue::encode() const {
    if (isInteger()) {
        return "i" + std::to_string(asInteger()) + "e";
    }
    else if (isString()) {
        const auto& str = asString();
        return std::to_string(str.length()) + ":" + str;
    }
    else if (isList()) {
        std::string result = "l";
        for (const auto& item : asList()) {
            result += item->encode();
        }
        return result + "e";
    }
    else if (isDict()) {
        std::string result = "d";
        for (const auto& [key, value] : asDict()) {
            result += std::to_string(key.length()) + ":" + key;
            result += value->encode();
        }
        return result + "e";
    }
    throw std::runtime_error("Invalid bencode value type");
}

std::shared_ptr<BencodeValue> BencodeParser::parse(const std::string& data) {
    size_t pos = 0;
    return parseValue(data, pos);
}

std::shared_ptr<BencodeValue> BencodeParser::parseFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string data((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    return parse(data);
}

std::shared_ptr<BencodeValue> BencodeParser::parseValue(const std::string& data, size_t& pos) {
    if (pos >= data.length()) {
        throw std::runtime_error("Unexpected end of data");
    }
    
    char c = data[pos];
    switch (c) {
        case 'i': return std::make_shared<BencodeValue>(parseInteger(data, pos));
        case 'l': return std::make_shared<BencodeValue>(parseList(data, pos));
        case 'd': return std::make_shared<BencodeValue>(parseDict(data, pos));
        default:
            if (std::isdigit(c)) {
                return std::make_shared<BencodeValue>(parseString(data, pos));
            }
            throw std::runtime_error("Invalid bencode format");
    }
}

BencodeInteger BencodeParser::parseInteger(const std::string& data, size_t& pos) {
    if (data[pos] != 'i') {
        throw std::runtime_error("Expected 'i' for integer");
    }
    pos++; // Skip 'i'
    
    size_t end = data.find('e', pos);
    if (end == std::string::npos) {
        throw std::runtime_error("Missing 'e' for integer");
    }
    
    BencodeInteger value = std::stoll(data.substr(pos, end - pos));
    pos = end + 1; // Skip 'e'
    return value;
}

BencodeString BencodeParser::parseString(const std::string& data, size_t& pos) {
    size_t colon = data.find(':', pos);
    if (colon == std::string::npos) {
        throw std::runtime_error("Missing ':' in string");
    }
    
    size_t length = std::stoul(data.substr(pos, colon - pos));
    pos = colon + 1;
    
    if (pos + length > data.length()) {
        throw std::runtime_error("String length exceeds data length");
    }
    
    BencodeString result = data.substr(pos, length);
    pos += length;
    return result;
}

BencodeList BencodeParser::parseList(const std::string& data, size_t& pos) {
    if (data[pos] != 'l') {
        throw std::runtime_error("Expected 'l' for list");
    }
    pos++; // Skip 'l'
    
    BencodeList result;
    while (pos < data.length() && data[pos] != 'e') {
        result.push_back(parseValue(data, pos));
    }
    
    if (pos >= data.length() || data[pos] != 'e') {
        throw std::runtime_error("Missing 'e' for list");
    }
    pos++; // Skip 'e'
    
    return result;
}

BencodeDict BencodeParser::parseDict(const std::string& data, size_t& pos) {
    if (data[pos] != 'd') {
        throw std::runtime_error("Expected 'd' for dictionary");
    }
    pos++; // Skip 'd'
    
    BencodeDict result;
    while (pos < data.length() && data[pos] != 'e') {
        BencodeString key = parseString(data, pos);
        auto value = parseValue(data, pos);
        result[key] = value;
    }
    
    if (pos >= data.length() || data[pos] != 'e') {
        throw std::runtime_error("Missing 'e' for dictionary");
    }
    pos++; // Skip 'e'
    
    return result;
}

} // namespace bencode 