//
// Created by YongGyu Lee on 2022/06/14.
//

#ifndef ASIO_SERVER_STORAGE_MANAGER_H_
#define ASIO_SERVER_STORAGE_MANAGER_H_

#include <fstream>
#include <string>
#include <optional>
#include <utility>

#define STRINGFY_IMPL(x) #x
#define STRINGFY(x) STRINGFY_IMPL(x)

inline const char* kPwd = STRINGFY(STREAM_SERVER_BASE_DIR);

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
std::string Stringfy(T val) { return std::to_string(val); }

std::string Stringfy(char val) = delete;

template<typename T, std::enable_if_t<std::is_constructible_v<std::string, T&&>, int> = 0>
std::string Stringfy(T&& val) { return std::string(std::forward<T>(val)); }

template<typename T, typename = void>
struct convertible_to_string : std::false_type {};
template<typename T>
struct convertible_to_string<T, std::void_t<decltype(Stringfy(std::declval<T&>()))>> : std::true_type {};

template<typename Str, typename ...Args,
  std::enable_if_t<
    std::conjunction_v<convertible_to_string<Str&&>,
                       convertible_to_string<Args&&>...
    >, int> = 0>
inline std::string StrCat(Str&& a, Args&&... b) {
  return (Stringfy(std::forward<Str>(a)) + ... + Stringfy(std::forward<Args>(b)));
}

class InvalidStorageAccess : public std::exception {
 public:
  InvalidStorageAccess(std::string access_point)
    : access_point_("Invalid access to " + std::move(access_point)) {}

  virtual ~InvalidStorageAccess() = default;

  const char *what() const noexcept override {
    return access_point_.c_str();
  }

 private:
  std::string access_point_;
};

class Storage {
 public:
  enum Code {
    kSuccess = 1,
    kInvalidAccess,
    kError
  };

  explicit Storage(std::string root);

  [[nodiscard]] std::optional<std::string> Read(const std::string& path, std::ios_base::openmode mode = std::ios::out) const;

  void Write(const std::string& path, const char* data, size_t size, std::ios_base::openmode mode = std::ios_base::out) const;

  void Remove(const std::string& path) const;

  void Rename(const std::string& old_path, const std::string& new_path) const;

  const std::string& root() const { return root_; }
  void root(std::string path) { root_ = std::move(path); }

  [[nodiscard]] Storage SubStorage(const std::string& path) const;

 private:
  bool valid_path(const std::string& target) const;

  void check_path(const std::string& target) const;

  std::string root_;
};

class StorageManager {
 public:
  static StorageManager& Get();

  enum Location {
    kPublic,
    kPrivate,
  };

  [[nodiscard]] const Storage& storage(Location loc) const;

  [[nodiscard]] const Storage& public_storage() const { return public_storage_; }
  [[nodiscard]] const Storage& private_storage() const { return private_storage_; }

  void relocate(const std::string& path);

  [[nodiscard]] std::optional<std::string>
    Read(Location loc, const std::string& path,
         std::ios_base::openmode mode = std::ios_base::out) const;

  void Write(Location loc, const std::string& path,
             const char* data, size_t size,
             std::ios_base::openmode mode = std::ios_base::out) const;

  void Remove(Location loc, const std::string& path) const;

  void Rename(Location loc, const std::string& old_path, const std::string& new_path) const;

 private:
  StorageManager();

  Storage base_storage_;
  Storage private_storage_;
  Storage public_storage_;
};

#endif // ASIO_SERVER_STORAGE_MANAGER_H_
