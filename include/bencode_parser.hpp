#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>

namespace bencode {

// Forward declarations
class BencodeValue;

// Type definitions for bencoded values
using BencodeInteger = int64_t;
using BencodeString = std::string;
using BencodeList = std::vector<std::shared_ptr<BencodeValue>>;
using BencodeDict = std::map<std::string, std::shared_ptr<BencodeValue>>;

// Main bencode value class that can hold any bencoded type
class BencodeValue {
public:
    using ValueType = std::variant<BencodeInteger, BencodeString, BencodeList, BencodeDict>;
    
    explicit BencodeValue(const ValueType& value) : value_(value) {}
    
    // Type checking methods
    bool isInteger() const { return std::holds_alternative<BencodeInteger>(value_); }
    bool isString() const { return std::holds_alternative<BencodeString>(value_); }
    bool isList() const { return std::holds_alternative<BencodeList>(value_); }
    bool isDict() const { return std::holds_alternative<BencodeDict>(value_); }
    
    // Value access methods
    BencodeInteger asInteger() const { return std::get<BencodeInteger>(value_); }
    const BencodeString& asString() const { return std::get<BencodeString>(value_); }
    const BencodeList& asList() const { return std::get<BencodeList>(value_); }
    const BencodeDict& asDict() const { return std::get<BencodeDict>(value_); }
    
    // Encode the value back to bencoded string
    std::string encode() const;
    
private:
    ValueType value_;
};

class BencodeParser {
public:
    static std::shared_ptr<BencodeValue> parse(const std::string& data);
    static std::shared_ptr<BencodeValue> parseFile(const std::string& filename);
    
private:
    static std::shared_ptr<BencodeValue> parseValue(const std::string& data, size_t& pos);
    static BencodeInteger parseInteger(const std::string& data, size_t& pos);
    static BencodeString parseString(const std::string& data, size_t& pos);
    static BencodeList parseList(const std::string& data, size_t& pos);
    static BencodeDict parseDict(const std::string& data, size_t& pos);
};

} // namespace bencode 