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

using boost::asio::ip::tcp;

constexpr const char* kPwd = "/home/ubuntu/Documents/GitHub/asio-server/";

std::string get_build_type()
{
#ifdef NDEBUG
  return "Release";
#else
  return "Debug";
#endif
}

void update_index_file(int idx) {
  static constexpr const char* kTempFileName = "last_index.tmp.txt";
  static constexpr const char* kTargetFileName = "last_index.txt";

  std::ofstream ofs;
  ofs.open(kPwd + std::string("images/") + kTempFileName);

  if (!ofs.is_open())
    return;

  const auto value = std::to_string(idx);
  ofs.write(value.c_str(), value.size());
  ofs.close();

  namespace fs = std::filesystem;
  fs::rename(fs::path(kPwd + std::string("images/") + kTempFileName),
             fs::path(kPwd + std::string("images/") + kTargetFileName));
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
//     ofs.open(kPwd + file_name);
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

std::vector<char> listen(boost::asio::io_context& io_context, tcp::acceptor& acceptor) {
  std::vector<char> data;
  Packet packet;

  bool get = false;
  static int idx = 1;
  std::ofstream ofs;

  for(;;) {
    tcp::socket socket(io_context);
    acceptor.accept(socket);

    boost::system::error_code error;
    const std::size_t len = boost::asio::read(socket, boost::asio::buffer(packet.buffer(), kPacketSize), error);
    packet.setSize(len);

    const auto header = packet.header();
    const auto it = header.find(kHeaderDone);
    const auto done = it != header.end() && it->second == "1";

    for (const auto& p : header) {
      std::cout << p.first << ": " << p.second << std::endl;
    }
    std::cout << "DataSize: " << len << std::endl;

    if (const auto rq = header.find(kRequest); rq->second == kRequestGet) {
      get = true;
    }

    if (const auto fmt = header.find("FileFormat"); fmt != header.end()) {
      const auto path = kPwd + std::string("images/") + std::to_string(idx) + ".jpg";
      std::cout << "Path: " << path << '\n';

      if (!ofs.is_open()) {
        ofs.open(path, std::ios::binary);
      }
      const auto data = packet.data();
      ofs.write(data.first, len);
    }

    if (error == boost::asio::error::eof) {
      std::cout << "EOF: Connection closed cleanly by peer." << std::endl;
      break;
    }
    if (done) {
      std::cout << "File send done." << std::endl;
      break;
    }
  }

  if (ofs.is_open()) {
    ofs.close();
    update_index_file(idx);
    ++idx;
  }

  if (get) {
    const auto data_view = packet.data();
    const auto data = std::string(data_view.first, data_view.second);
    std::cout << "Client requested: " << data << '\n';
    
    std::ifstream ifs;
    ifs.open(std::string(kPwd) + data);
    if (ifs.is_open()) {
      std::stringstream buffer;
      buffer << ifs.rdbuf();
      const auto file_content = buffer.str();

      const char* data = file_content.c_str();
      const auto data_size = file_content.size();

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

  return data;
}

int main(int argc, char* argv[])
{
  if(argc != 2) {
    std::cerr << "port number not provided\n";
    return EXIT_FAILURE;
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

          
    boost::array<char, kPacketSize> buf{};
    boost::system::error_code error;
    std::size_t len;

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
