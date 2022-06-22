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
#include <boost/range/algorithm/fill.hpp>

namespace {
void main_coroutine(boost::asio::io_context &io_context,
                    boost::asio::ip::tcp::endpoint connect_addr,
                    const std::size_t total_send_length,
                    boost::asio::yield_context yield) try {
  boost::asio::ip::tcp::socket tcp_stream{io_context};
  tcp_stream.async_connect(connect_addr, yield);
  std::clog << "Connected to " << tcp_stream.remote_endpoint() << std::endl;

  constexpr std::size_t mtu = 1400;
  const std::string send_buf = [] {
    std::string ret;
    ret.resize(mtu);
    boost::range::fill(ret, 'a');
    return ret;
  }();
  const auto t1 = std::chrono::system_clock::now();
  for (std::size_t sent_len = 0; sent_len < total_send_length;
       sent_len += mtu) {
    boost::asio::async_write(tcp_stream, boost::asio::buffer(send_buf),
                             boost::asio::transfer_all(), yield);
  }
  const auto t2 = std::chrono::system_clock::now();
  std::clog << "Elapsed: " << std::chrono::duration<double>(t2 - t1).count()
            << " s" << std::endl;
} catch (const std::exception &ex) {
  std::cerr << "Error in main_coroutine(): " << ex.what() << std::endl;
}

} // namespace

int main(const int argc, const char *argv[]) try {
  if (argc < 4) {
    std::cerr << "Usage: client <ip_address> <port> <total_send_length>"
              << std::endl;
    return 1;
  }
  boost::asio::ip::tcp::endpoint connect_addr{
      boost::asio::ip::make_address(argv[1]),
      boost::lexical_cast<std::uint16_t>(argv[2])};
  const std::size_t total_send_length =
      boost::lexical_cast<std::size_t>(argv[3]);
  boost::asio::io_context io_context;
  boost::asio::spawn(io_context, [&io_context, connect_addr, total_send_length](
                                     boost::asio::yield_context yield) {
    main_coroutine(io_context, connect_addr, total_send_length, yield);
  });
  io_context.run();
} catch (const std::exception &ex) {
  std::cerr << "Error in main(): " << ex.what() << std::endl;
  return 1;
}
