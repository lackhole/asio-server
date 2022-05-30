#include <ctime>
#include <iostream>
#include <string>
#include <climits>
#include <exception>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

std::string get_build_type()
{
#ifdef NDEBUG
  return "Release";
#else
  return "Debug";
#endif
}

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(nullptr);
  return ctime(&now);
}

int main(int argc, char* argv[])
{
  if(argc != 2) {
    std::cerr << "port number not provided\n";
    return EXIT_FAILURE;
  }

  std::cout << "Build Type: " << get_build_type() << std::endl;

  try
  {
    int port_num = std::stoi(argv[1]);
    if(port_num >= USHRT_MAX)
      throw std::runtime_error("invalid port number: " + std::to_string(port_num));

    boost::asio::io_context io_context;
    auto endpoint = tcp::endpoint(tcp::v4(), port_num);

    tcp::acceptor acceptor(io_context, endpoint);
    std::cout << make_daytime_string() << " Running at " << endpoint << std::endl;

    do {
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::string message = make_daytime_string();
      std::cout << message << std::endl;

      boost::system::error_code ignored_error;
      boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
    } while (false);

    for (;;)
    {
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      boost::array<char, 128> buf{};
      boost::system::error_code error;

      std::size_t len = socket.receive(boost::asio::buffer(buf));

      if (error == boost::asio::error::eof)
        break;
      else if (error)
        throw boost::system::system_error(error);

      std::cout.write(buf.data(), len);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
