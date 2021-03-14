#include <ctime>
#include <iostream>
#include <string>
#include <climits>
#include <exception>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

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


  try
  {
    int port_num = std::stoi(argv[1]);
    if(port_num >= USHRT_MAX)
      throw std::runtime_error("invalid port number: " + std::to_string(port_num));

    std::cout << make_daytime_string() + " Initializing\n";

    boost::asio::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port_num));
    std::cout << make_daytime_string() + " Running\n";
    for (;;)
    {
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::string message = make_daytime_string();
      std::cout << message << std::endl;

      boost::system::error_code ignored_error;
      boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}