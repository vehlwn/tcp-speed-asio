#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

#include <boost/asio/buffer.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>

namespace {
void handle_client(boost::asio::io_context &io_context,
                   boost::asio::ip::tcp::socket tcp_stream,
                   boost::asio::yield_context yield) try {
  constexpr std::size_t tcp_recv_size = 4096;
  std::string buf;
  while (true) {
    buf.resize(tcp_recv_size);
    boost::system::error_code ec;
    const std::size_t tcp_read_len =
        tcp_stream.async_receive(boost::asio::buffer(buf), yield[ec]);
    if (ec) {
      if (ec == boost::asio::error::eof)
        break;
      else
        throw boost::system::system_error{ec};
    }
  }
} catch (const std::exception &ex) {
  std::cerr << "Error in handle_client(): " << ex.what() << std::endl;
}

void main_coroutine(boost::asio::io_context &io_context,
                    boost::asio::ip::tcp::endpoint listen_addr,
                    boost::asio::yield_context yield) try {
  boost::asio::ip::tcp::acceptor tcp_listener{io_context};
  tcp_listener.open(listen_addr.protocol());
  tcp_listener.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  tcp_listener.bind(listen_addr);
  tcp_listener.listen();
  std::clog << "Listening on " << tcp_listener.local_endpoint() << "/TCP"
            << std::endl;

  while (true) {
    boost::asio::ip::tcp::socket tcp_stream{io_context};
    tcp_listener.async_accept(tcp_stream, yield);
    std::clog << "Incoming connection from " << tcp_stream.remote_endpoint()
              << std::endl;
    boost::asio::spawn(
        io_context, [&io_context, tcp_stream = std::move(tcp_stream)](
                        boost::asio::yield_context yield) mutable {
          handle_client(io_context, std::move(tcp_stream), yield);
        });
  }
} catch (const std::exception &ex) {
  std::cerr << "Error in main_coroutine(): " << ex.what() << std::endl;
}

} // namespace

int main(const int argc, const char *argv[]) try {
  if (argc < 3) {
    std::cerr << "Usage: server <ip_address> <port>" << std::endl;
    return 1;
  }
  boost::asio::ip::tcp::endpoint listen_addr{
      boost::asio::ip::make_address(argv[1]),
      boost::lexical_cast<std::uint16_t>(argv[2])};
  boost::asio::io_context io_context;
  boost::asio::spawn(
      io_context, [&io_context, listen_addr](boost::asio::yield_context yield) {
        main_coroutine(io_context, listen_addr, yield);
      });
  io_context.run();
} catch (const std::exception &ex) {
  std::cerr << "Error in main(): " << ex.what() << std::endl;
  return 1;
}
