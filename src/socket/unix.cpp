module;

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cerrno>
#include <format>
#include <optional>
#include <string>
#include <system_error>
#include <unistd.h>

module socket;

/*
  Implementation. 
  Do not read if you are nto interested
*/
/*
  Begin for Unix TCP Listener
*/

/*
  Constructor and Destructor Pair. 
  Constructor : connects and listen on the port. This follows the RAII principle. 
  Hence, the destructor will be written right after this.
*/
sock::UnixTCPListener::UnixTCPListener(
  std::string addr, int port_number, int back_log
) :
  _port_number(port_number),
  _address(addr == "localhost" ? "127.0.0.1" : addr), _back_log(back_log) {
  // Sys Call to create socket
  _sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (_sock_fd == -1) _throw_exception("cannot create socket");

  // Bind socket to the specified address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(_port_number);
  server_address.sin_addr.s_addr = inet_addr(_address.c_str());

  if (bind(_sock_fd, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) == -1)
    _throw_exception(std::format("Cannot bind at {}:{}", _address, _port_number)
    );

  // Listen
  if (listen(_sock_fd, _back_log) == -1)
    _throw_exception(
      std::format("fail to listen at {}:{}", _address, _port_number)
    );
  // Done !
}
sock::UnixTCPListener::~UnixTCPListener() {
  if (_sock_fd != -1) close(_sock_fd);
}
// Move Constructor
sock::UnixTCPListener::UnixTCPListener(sock::UnixTCPListener &&listener) :
  _port_number(listener._port_number), _address(listener._address) {
  _sock_fd = listener._sock_fd;
  // Invalidate the previous one
  listener._sock_fd = -1;
  // Since it must be listening, then we can skip the initialization process
}
// Move Assignment
sock::UnixTCPListener &sock::UnixTCPListener::operator=(
  sock::UnixTCPListener &&listener
) {
  /*
    Close the current socket with RAII. Move the current socket to
    current_socket. This will invalidate the current _sock_fd and address
    and move it to current_socket
  */
  sock::UnixTCPListener current_socket{std::move(*this)};
  /*
    Move contents from the other sockets. This will make sure that the RAII
    on the other socket does not conflict with the current one. 
  */
  this->_port_number = listener._port_number;
  this->_address = listener._address;
  this->_sock_fd = listener._sock_fd;
  listener._sock_fd = -1;
  return *this;
}

/*
  Accept connection and create a TCP Connection Handler
*/
std::optional<sock::UnixTCPHandler> sock::UnixTCPListener::accept() noexcept {
  struct sockaddr_in client_address;
  unsigned int client_address_length = sizeof(client_address);
  int accepted_sock_fd = ::accept(
    _sock_fd,
    reinterpret_cast<struct sockaddr *>(&client_address),
    &client_address_length
  );
  if (accepted_sock_fd == -1) {
    return std::nullopt;
  } else {
    std::string ipv4_address = inet_ntoa(client_address.sin_addr);
    return sock::UnixTCPHandler{accepted_sock_fd, ipv4_address};
  }
}

/*
  Unix TCP Handler.
*/
sock::UnixTCPHandler::UnixTCPHandler(int sock_fd, std::string address) :
  _sock_fd(sock_fd), _address(std::move(address)) {}
sock::UnixTCPHandler::~UnixTCPHandler() {
  if (_sock_fd != -1){
    close(_sock_fd);
  }
}
sock::UnixTCPHandler::UnixTCPHandler(sock::UnixTCPHandler &&handler) {
  _address = std::move(handler._address);
  _sock_fd = handler._sock_fd;

  // Invalidate previous handler
  handler._sock_fd = -1;
}
sock::UnixTCPHandler &sock::UnixTCPHandler::operator=(
  sock::UnixTCPHandler &&handler
) {
  /*
    RAII control move the current socket to the context of this function
  */
  sock::UnixTCPHandler current_socket = std::move(*this);
  this->_sock_fd = handler._sock_fd;
  this->_address = std::move(handler._address);
  handler._sock_fd = -1;
  return *this;
}
void sock::UnixTCPHandler::send(const std::string &msg) {
  if (::send(_sock_fd, static_cast<const void *>(msg.c_str()), msg.length(), 0) != -1) {
    return;
  }
  /* 
    Error Handling
  */
  // Slice up message into smaller bits before sending
  if (errno == EMSGSIZE) {

  } else {
    throw std::system_error{std::error_code{errno, std::generic_category()}};
  }
}

std::string sock::UnixTCPHandler::recv(uint32_t read_count) {
  // Initialize buffer that will be finally used for return
  std::string buffer;
  buffer.reserve(read_count);
  uint32_t recv_count = read_count;

  // Initialize Buffer to use every iterations
  uint32_t per_iter_buf_size =
    read_count > default_buffer_size ? default_buffer_size : read_count;
  std::vector<char> temp_buffer;
  temp_buffer.resize(per_iter_buf_size);

  // Iterate till we get enough amount of data
  while (recv_count != 0) {
    uint32_t amount_to_read =
      recv_count > default_buffer_size ? default_buffer_size : recv_count;
    int amount_read = ::recv(
      _sock_fd, static_cast<void *>(temp_buffer.data()), amount_to_read, 0
    );
    if (amount_read == -1) {
      throw std::system_error{std::error_code{errno, std::generic_category()}};
    }
    recv_count -= amount_read;
    std::copy(
      temp_buffer.begin(),
      temp_buffer.begin() + amount_read,
      std::back_inserter(buffer)
    );

    // No more data to read if amount_read is less than requested
    if (amount_read < amount_to_read) break;
  }
  return buffer;
}
std::string sock::UnixTCPHandler::recv() {
    // Initialize buffer that will be finally used for return
  std::string buffer;
  char temp_buffer[default_buffer_size + 1];
  // Iterate until there is no data to take
  while (true) {
    int amount_read = ::recv(
      _sock_fd, static_cast<void *>(temp_buffer), default_buffer_size, 0
    );
    if (amount_read == -1) {
      throw std::system_error{std::error_code{errno, std::generic_category()}};
    }
    std::copy(
      temp_buffer, temp_buffer + amount_read,
      std::back_inserter(buffer)
    );
    // No more data to read if amount_read is less than requested
    if (amount_read < default_buffer_size) break;
  }
  return buffer;
}

std::string sock::UnixTCPHandler::recv_until(char x) {
  std::string buffer;
  char current_char = 0;
  while (true) {
    int amount_read = ::recv(_sock_fd, static_cast<void*>(&current_char), 1, 0);
    if (amount_read == -1)
      throw std::system_error{std::error_code{errno, std::generic_category()}};
    if (current_char == x)
      break;
    if (amount_read == 0)
      break;
    buffer.push_back(current_char);
  }
  return buffer;
}

int sock::UnixTCPHandler::unique_id() const {
  return _sock_fd;
}