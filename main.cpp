#include <ctime>
#include <cmath>
#include <iostream>
#include <string>
#include <climits>
#include <exception>
#include <fstream>
#include <sstream>
#include <thread>
#include <filesystem>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "packet.h"
#include "storage_manager.h"

using boost::asio::ip::tcp;

std::string get_build_type()
{
#ifdef NDEBUG
  return "Release";
#else
  return "Debug";
#endif
}

namespace fs = std::filesystem;

void update_index_file(int idx) {
  static constexpr const char* kTempFileName = "last_index.tmp.txt";
  static constexpr const char* kTargetFileName = "last_index.txt";

  const auto& storage = StorageManager::Get().public_storage();

  const auto value = std::to_string(idx);

  storage.Write(fs::path("images")/kTempFileName, value.c_str(), value.size());
  storage.Rename(fs::path("images")/kTempFileName, fs::path("images")/kTargetFileName);
}

std::string make_index(int value, int n_zero = 10) {
  const auto old_str = std::to_string(value);
  return std::string(n_zero - std::min<int>(n_zero, old_str.size()), '0') + old_str;
}

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(nullptr);
  return ctime(&now);
}


//   std::cout << "Writing file...\n";
//   boost::array<char, kPacketSize> buf{};
//   std::ofstream ofs;
//   ss.str("");

//   if (!ofs.is_open()) {
//     ofs.open(StrCat(kPwd, "/", file_name));
//   }

//   size_t index = 0;

//   for(;;) {
//     tcp::socket socket(io_context);
//     acceptor.accept(socket);
      
//     boost::system::error_code error;
//     const std::size_t len = socket.read(boost::asio::buffer(buf), error);
//     const std::size_t len = boost::asio::read(socket, boost::asio::buffer(buf), error);
//     std::cerr << ++index << " | Received " << len << "bytes." << std::endl;
//     ofs.write(buf.data(), len);
//     ss.write(buf.data(), len);

//     if (error == boost::asio::error::eof) {
//       std::cout << "EOF: Connection closed cleanly by peer." << std::endl;
//       break;
//     } else if (error) {
//       throw boost::system::system_error(error);
//     }

//     std::cout << std::string(buf.data(), len);
//   }
//   std::cerr << "\nWriting done" << std::endl;
// }

void listen(boost::asio::io_context& io_context, tcp::acceptor& acceptor) {
  Packet packet;

  bool get = false;
  static int idx = 1;
  boost::system::error_code error;

  const auto& public_storage = StorageManager::Get().public_storage();
  const auto& private_storage = StorageManager::Get().private_storage();

  for(;;) {
    tcp::socket socket(io_context);
    acceptor.accept(socket);

    const std::size_t len = boost::asio::read(socket, boost::asio::buffer(packet.buffer(), kPacketSize), error);
    packet.setSize(len);

    if (len >= to_byte(kPacketHeaderSizeBit)) {

      const auto header = packet.header();
      const auto it = header.find(kHeaderDone);
      const auto done = it != header.end() && it->second == "1";

      for (const auto& p: header) {
        std::cout << p.first << ": " << p.second << std::endl;
      }
      std::cout << "DataSize: " << len << std::endl;

      if (const auto rq = header.find(kRequest); rq->second == kRequestGet) {
        get = true;
      }

      if (const auto fmt = header.find("FileFormat"); fmt != header.end()) {
        const auto path = fs::path("images")/StrCat(idx, ".jpg");
        std::cout << "Path: " << path << '\n';

        const auto data = packet.data();
        public_storage.Write(path, data.first, len, std::ios::out | std::ios_base::app);
      }

      if (done) {
        std::cout << "File receive done." << std::endl;
        update_index_file(idx);
        ++idx;
        break;
      }
    } else if (error == boost::asio::error::eof) {
      public_storage.Remove(fs::path("images")/StrCat(idx, ".jpg"));
      std::cout << "EOF: Connection closed cleanly by peer." << std::endl;
      break;
    }
  }

  if (get) {
    const auto data_view = packet.data();
    const auto requested_file = std::string(data_view.first, data_view.second);
    std::cout << "Client requested: " << requested_file << '\n';

    const auto file_content = private_storage.Read(requested_file);
    if (!file_content) {
      std::cout << private_storage.root() + "/" + requested_file << " not exists.\n";

      // TODO: Send error code instead of manually closing
      tcp::socket socket(io_context);
      acceptor.accept(socket);
      socket.close();

      return;
    }

    std::cout << "Found " << file_content->c_str() << '\n';

    const char* data = file_content->c_str();
    const auto data_size = file_content->size();

    size_t sent_size_data = 0;
    size_t remaining_size = data_size;
    size_t sent_size_packet = 0;

    while (remaining_size > 0) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);


      // TODO: Send some header values only once at the beginning
      std::unordered_map<std::string, std::string> header = {
        {kHeaderStatus, "200"},
        {kHeaderTotalSize, std::to_string(data_size)},
        {kHeaderDone, "0"}
      };

      const auto header_size = Packet::CalcHeaderSize(header);
      header[kHeaderDone] = std::to_string((header_size + remaining_size) <= packet.capacity());

      packet.clear();
      packet.write_header(header);


      const auto sending_size = packet.remaining_size() > remaining_size ? remaining_size : packet.remaining_size();

      packet.write_data(data + sent_size_data, sending_size);

      boost::system::error_code ignored_error;
      boost::asio::write(socket, boost::asio::buffer(packet.buffer(), packet.size()), ignored_error);

      remaining_size -= sending_size;
      sent_size_data += sending_size;
      sent_size_packet += packet.size();
      std::cout << "Sent " << sending_size << "bytes. (" << sending_size << '/' << data_size << ")\n";
    }
  }
}

int main(int argc, char* argv[])
{
  if(argc < 2) {
    std::cerr << "port number not provided\n";
    return EXIT_FAILURE;
  }

  if (argc == 3) {
    StorageManager::Get().relocate(argv[2]);
  }

  std::cout << "Build Type: " << get_build_type() << std::endl;

  update_index_file(1);

  try {
    int port_num = std::stoi(argv[1]);
    if(port_num >= USHRT_MAX)
      throw std::runtime_error("invalid port number: " + std::to_string(port_num));

    boost::asio::io_context io_context;
    auto endpoint = tcp::endpoint(tcp::v4(), port_num);

    tcp::acceptor acceptor(io_context, endpoint);
    std::cout << make_daytime_string() << " Running at " << endpoint << std::endl;

    for (;;) {
      std::cout << "<BEGIN>" << std::endl;
      listen(io_context, acceptor);
      std::cout << "<END>" << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
