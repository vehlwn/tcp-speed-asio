#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/asio/buffer.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>

namespace {
void main_coroutine(boost::asio::io_context &io_context,
                    boost::asio::ip::tcp::endpoint connect_addr,
                    const std::size_t total_recv_length,
                    boost::asio::yield_context yield) try {
  boost::asio::ip::tcp::socket tcp_stream{io_context};
  tcp_stream.async_connect(connect_addr, yield);
  std::clog << "Connected to " << tcp_stream.remote_endpoint() << std::endl;

  constexpr std::size_t mtu = 1400;
  std::string buf;
  const auto t1 = std::chrono::system_clock::now();
  for (std::size_t recv_len = 0; recv_len < total_recv_length;
       recv_len += mtu) {
    buf.resize(mtu);
    boost::system::error_code ec;
    const std::size_t tcp_read_len =
        tcp_stream.async_receive(boost::asio::buffer(buf), yield[ec]);
  }
  const auto t2 = std::chrono::system_clock::now();
  const double secs = std::chrono::duration<double>(t2 - t1).count();
  const double kbs = total_recv_length / 1024.0 / secs;
  const double mbs = kbs / 1024.0;
  std::clog << "Elapsed: " << secs << " s; " << kbs << " KB/s; " << mbs
            << " MB/s" << std::endl;
} catch (const std::exception &ex) {
  std::cerr << "Error in main_coroutine(): " << ex.what() << std::endl;
}

} // namespace

int main(const int argc, const char *argv[]) try {
  if (argc < 4) {
    std::cerr << "Usage: client <ip_address> <port> <total_recv_length>"
              << std::endl;
    return 1;
  }
  boost::asio::ip::tcp::endpoint connect_addr{
      boost::asio::ip::make_address(argv[1]),
      boost::lexical_cast<std::uint16_t>(argv[2])};
  const std::size_t total_recv_length =
      boost::lexical_cast<std::size_t>(argv[3]);
  boost::asio::io_context io_context;
  boost::asio::spawn(io_context, [&io_context, connect_addr, total_recv_length](
                                     boost::asio::yield_context yield) {
    main_coroutine(io_context, connect_addr, total_recv_length, yield);
  });
  io_context.run();
} catch (const std::exception &ex) {
  std::cerr << "Error in main(): " << ex.what() << std::endl;
  return 1;
}
