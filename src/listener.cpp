#include "expresscpp/impl/listener.hpp"
namespace expresscpp {

Listener::Listener(boost::asio::ip::tcp::endpoint endpoint, ExpressCpp *express_cpp,
                   ready_fn_cb_error_code_t error_callback)
    : acceptor_(net::make_strand(ioc_)), express_cpp_(express_cpp), io_threads(threads_) {
  assert(express_cpp_ != nullptr);

  boost::beast::error_code ec;

  // Open the acceptor
  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    error_callback(ec);
    fail(ec, "open");
    return;
  }

  // Allow address reuse
  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if (ec) {
    error_callback(ec);
    fail(ec, "set_option");
    return;
  }

  // Bind to the server address
  acceptor_.bind(endpoint, ec);
  if (ec) {
    error_callback(ec);
    fail(ec, "bind");
    return;
  }

  // Start listening for connections
  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    error_callback(ec);
    fail(ec, "listen");
    return;
  }
}

void Listener::run() {
  do_accept();
}

void Listener::do_accept() {
  // The new connection gets its own strand
  acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&Listener::on_accept, shared_from_this()));
}

void Listener::on_accept(beast::error_code ec, tcp::socket socket) {
  if (ec) {
    fail(ec, "accept");
  } else {
    // Create the session and run it
    std::make_shared<Session>(std::move(socket), express_cpp_)->run();
  }

  // Accept another connection
  do_accept();
}

}  // namespace expresscpp
