//
// Created by YongGyu Lee on 2022/06/14.
//

#include "stream/storage_manager.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace fs = std::filesystem;

inline constexpr const char* const kPathPublic = "public";
inline constexpr const char* const kPathPrivate = "private";

static bool CheckPath(fs::path check, const fs::path& base) {
  for (;;) {
    if (check == base)
      return true;

    if (!check.has_parent_path())
      return false;

    check = check.parent_path();
  }
}

template<typename F> void assign_if(F func) { func(); }
template<typename F, typename R> void assign_if(F func, R& r) { r = func(); }

template<typename F, typename ...Out>
std::enable_if_t<
  std::conjunction_v<
    std::is_invocable<F>,
    std::is_convertible<std::invoke_result_t<F>, Out>...
  >>
CatchException(F func, Out&... o) {
  try {
    assign_if(func, o...);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}

Storage::Storage(std::string root) : root_(std::move(root)) {}

std::optional<std::string> Storage::Read(const std::string& path, std::ios_base::openmode mode) const {
  const auto p = fs::path(root())/path;
  check_path(p);

  std::ifstream ifs;
  ifs.open(p, mode);
  if (!ifs.is_open()) {
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << ifs.rdbuf();
  return buffer.str();
}

void Storage::Write(const std::string& path, const char* data, size_t size, std::ios_base::openmode mode) const {
  const auto p = fs::path(root())/path;
  check_path(p);


  const auto do_write = [&]() {
    std::ofstream ofs;
    if (ofs.open(p, mode); ofs.is_open()) {
      ofs.write(data, size);
      return true;
    }
    return false;
  };

  if (do_write())
    return;

  if (const auto ppath = p.parent_path(); !fs::exists(ppath)) {
    fs::create_directories(ppath);
  }

  do_write();
}

void Storage::Remove(const std::string& path) const {
  const auto p = fs::path(root())/path;
  check_path(p);

  fs::remove_all(p);
}

void Storage::Rename(const std::string& old_path, const std::string& new_path) const {
  const auto p_old = fs::path(root())/old_path;
  const auto p_new = fs::path(root())/new_path;

  check_path(p_old);
  check_path(p_new);

  fs::rename(p_old, p_new);
}

bool Storage::valid_path(const std::string& target) const {
  return CheckPath(target, root_);
}

void Storage::check_path(const std::string& target) const {
  if (!valid_path(target))
    throw InvalidStorageAccess(target);
}

Storage Storage::SubStorage(const std::string& path) const {
  const auto p = fs::path(root())/path;
  check_path(p);
  return Storage(p);
}

StorageManager& StorageManager::Get() {
  static auto inst = new StorageManager();
  return *inst;
}

StorageManager::StorageManager()
  : base_storage_   (fs::path(kPwd)/"storage"),
    private_storage_(base_storage_.SubStorage(kPathPrivate)),
    public_storage_ (base_storage_.SubStorage(kPathPublic))
{}

std::optional<std::string> StorageManager::Read(Location loc, const std::string& path,
                                                std::ios_base::openmode mode) const {
  std::optional<std::string> result;
  CatchException([&](){ return storage(loc).Read(path, mode); }, result);
  return result;
}

void StorageManager::Write(Location loc, const std::string& path,
                           const char* data, size_t size,
                           std::ios_base::openmode mode) const {
  CatchException([&](){ return storage(loc).Write(path, data, size, mode); });
}

void StorageManager::Remove(StorageManager::Location loc, const std::string& path) const {
  CatchException([&](){ return storage(loc).Remove(path); });
}

void StorageManager::Rename(Location loc, const std::string& old_path, const std::string& new_path) const {
  CatchException([&](){ return storage(loc).Rename(old_path, new_path); });
}

const Storage& StorageManager::storage(Location loc) const {
  return loc == kPublic ? public_storage_ : private_storage_;
}

void StorageManager::relocate(const std::string& path) {
  base_storage_.root(path);
  public_storage_ = base_storage_.SubStorage(kPathPublic);
  private_storage_ = base_storage_.SubStorage(kPathPrivate);
}
