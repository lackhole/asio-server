//
// Created by YongGyu Lee on 2022/06/11.
//

#ifndef EMBED_NETWORK_PACKET_H_
#define EMBED_NETWORK_PACKET_H_

#include <cstdint>
#include <cstring>
#include <climits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

enum : size_t {
  kPacketSize = 1'000'000, // 1MB
  kPacketHeaderSizeBit = 32,
};

inline constexpr const char* kRequest = "REQUEST";
inline constexpr const char* kRequestPost = "POST";
inline constexpr const char* kRequestGet = "GET";

inline constexpr const char* kHeaderTotalSize = "TotalSize";
inline constexpr const char* kHeaderFileName = "FileName";
inline constexpr const char* kHeaderFileFormat = "FileFormat";
inline constexpr const char* kHeaderStreamIndex = "StreamIndex";
inline constexpr const char* kHeaderDone = "Done";
inline constexpr const char* kHeaderTime = "Time";
inline constexpr const char* kHeaderStatus = "Status";

template<typename T>
constexpr inline T to_byte(T bit) { return static_cast<T>(bit / CHAR_BIT); }

inline std::unordered_map<std::string_view, std::string_view>
tokenize(const char* str, size_t len,
         const char kv = '=',
         const char sep = ';') {
  std::unordered_map<std::string_view, std::string_view> tokens;

  std::string_view last_key;
  size_t last_idx = 0;
  const auto padding = to_byte(kPacketHeaderSizeBit);

  size_t i = 0;
  while (i < len) {
    if (str[i] == kv) {
      last_key = std::string_view(str + last_idx, i - last_idx);
      last_idx = i + 1;
    } else if (str[i] == sep /* || i + 1 == header.size() */) {
      tokens.emplace(
        last_key,
        std::string_view(str + last_idx, i - last_idx));
      last_idx = i + 1;
    }
    ++i;
  }

  return tokens;
}

class Packet {
 public:
  using value_type = char;
  using string_type = std::basic_string<value_type>;
  using string_view_type = std::basic_string_view<value_type>;

  explicit Packet(size_t packet_size = kPacketSize) : buffer_(packet_size) {}

  static uint32_t CalcHeaderSize(const std::unordered_map<std::string, std::string>& header) {
    uint32_t header_size = to_byte(kPacketHeaderSizeBit);
    for (const auto& p : header) {
      const auto& key = p.first;
      const auto& value = p.second;
      header_size += key.size() + value.size() + 2;
    }

    return header_size;
  }

  Packet& write_header(const std::unordered_map<std::string, std::string>& header) {
    const uint32_t header_size = CalcHeaderSize(header);

    if (header_size > kPacketSize)
      throw_length_error(header_size);

//    if (buffer_.size() < header_size)
//      buffer_.resize(header_size);

    char* pos = buffer() + to_byte(kPacketHeaderSizeBit);
    for (const auto& p : header) {
      const auto& key = p.first;
      const auto& value = p.second;

      std::memcpy(pos, key.data(), key.size()); // write key
      *(pos + key.size()) = '='; // write indicator
      std::memcpy(pos + key.size() + 1, value.data(), value.size()); // write value
      *(pos + key.size() + value.size() + 1) = ';'; // write separator
      pos += key.size() + value.size() + 2;
    }

    (*reinterpret_cast<uint32_t*>(buffer())) = header_size;
    content_size_ = header_size;

    return *this;
  }

  [[nodiscard]] size_t remaining_size() const noexcept {
    return kPacketSize - content_size_;
  }

  size_t write_data(const char* data, size_t data_size) {
    const size_t write_size = data_size > remaining_size() ? remaining_size() : data_size;

//    if (buffer_.size() < content_size_ + write_size)
//      buffer_.resize(content_size_ + write_size);

    std::memcpy(this->data().first, data, write_size);
    content_size_ += write_size;
    return write_size;
  }

  [[nodiscard]] std::unordered_map<string_view_type, string_view_type>
  header() const {
    const auto rh = raw_header();
    return tokenize(rh.data(), rh.size());
  }

  [[nodiscard]] std::pair<const char*, size_t> data() const noexcept {
    return {buffer() + raw_header_size(), size() - raw_header_size()};
  }
  [[nodiscard]] std::pair<char*, size_t> data() noexcept {
    return {buffer() + raw_header_size(), size() - raw_header_size()};
  }

  char* buffer() noexcept { return buffer_.data(); }
  [[nodiscard]] const char* buffer() const noexcept { return buffer_.data(); }

  [[nodiscard]] size_t size() const noexcept { return content_size_; }

  [[nodiscard]] size_t capacity() const noexcept { return kPacketSize; }

  void clear() {
    content_size_ = 0;
  }

  void setSize(size_t new_sz) {
    content_size_ = new_sz;
  }

  void moveTo(std::vector<char>& dst) {
    dst = std::move(buffer_);
  }

 private:
  void throw_length_error(size_t try_write_size) {
    throw std::runtime_error(
      std::string("Header size is too big (") + std::to_string(try_write_size) + "<=" + std::to_string(kPacketSize) + ")");
  }

  [[nodiscard]] uint32_t raw_header_size() const noexcept {
    static_assert(kPacketSize > kPacketHeaderSizeBit);
    return *reinterpret_cast<const uint32_t*>(buffer());
  }

  [[nodiscard]] string_view_type raw_header() const noexcept {
    return {buffer_.data() + to_byte(kPacketHeaderSizeBit),
            raw_header_size() - to_byte(kPacketHeaderSizeBit)};
  }

  // TODO: Make static
//  [[nodiscard]] std::unordered_map<string_view_type, string_view_type>
//  tokenize(char kv = '=', char sep = ';') const {
//    std::unordered_map<string_view_type, string_view_type> tokens;
//
//    const auto header = raw_header();
//    string_view_type last_key;
//    size_t last_idx = 0;
//    const auto padding = to_byte(kPacketHeaderSizeBit);
//
//    size_t i = 0;
//    while (i < header.size()) {
//      if (header[i] == kv) {
//        last_key = string_view_type(buffer_.data() + padding + last_idx, i - last_idx);
//        last_idx = i + 1;
//      } else if (header[i] == sep /* || i + 1 == header.size() */) {
//        tokens.emplace(
//          last_key,
//          string_view_type(buffer_.data() + padding + last_idx, i - last_idx));
//        last_idx = i + 1;
//      }
//      ++i;
//    }
//
//    return tokens;
//  }

  // TODO: Optimize storage
  std::vector<value_type> buffer_;
  size_t content_size_ = 0;
};

#endif // EMBED_NETWORK_PACKET_H_
