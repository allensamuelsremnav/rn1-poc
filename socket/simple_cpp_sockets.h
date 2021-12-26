// got this from https://github.com/computersarecool/cpp_sockets/tree/master/src


#include <string>
#include <iostream>
#include <exception>

#include <sys/socket.h>
#include <arpa/inet.h> // This contains inet_addr
#include <unistd.h> // This contains close
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
typedef int SOCKET;

// These could also be enums
struct bind_err : public std::exception { };
struct socket_err : public std::exception {};
struct accept_err : public std::exception {};
struct connection_err : public std::exception{};
struct send_err : public std::exception{};
struct recv_err : public std::exception{};

class Socket {
public:
    std::string recv();
    void send(const std::string&);
    ~Socket() { close(m_socket); }
    Socket& operator=(const Socket& s) {
        m_socket = s.m_socket;
        m_addr   = s.m_addr;
        const_cast<Socket&>(s).m_socket = 0;
        return *this;
    }
    Socket(const Socket& s) {
        m_socket = s.m_socket;
        m_addr   = s.m_addr;
        const_cast<Socket&>(s).m_socket = 0;
    }
    void bind();
    enum class SocketType {
        TYPE_STREAM = SOCK_STREAM,
        TYPE_DGRAM = SOCK_DGRAM
    };
    void set_port(u_short port) { m_addr.sin_port = htons(port); }
    int set_address(const std::string& ip_address) { return inet_pton(AF_INET, ip_address.c_str(), &m_addr.sin_addr); }
    explicit Socket(SocketType socket_type = SocketType::TYPE_DGRAM);
    Socket accept();
protected:
    explicit Socket(SOCKET socket, sockaddr_in addr) : m_socket(socket), m_addr(addr) {}
    SOCKET m_socket;
    sockaddr_in m_addr;
};

class TCPClient : public Socket
{
public:
    TCPClient(u_short port, const std::string& ip_address = "127.0.0.1");
    void make_connection();
};

class TCPServer
{
public:
    TCPServer(u_short port, const std::string& ip_address = "0.0.0.0");
    void bind() { m_listen.bind(); }
    Socket accept() { return m_listen.accept(); }
private:
    Socket m_listen;
};

Socket::Socket(const SocketType socket_type) : m_socket(), m_addr() { 
    // Create the socket handle
    m_socket = socket(AF_INET, static_cast<int>(socket_type), 0);
    if (m_socket == INVALID_SOCKET) {
        throw socket_err();
    }
    m_addr.sin_family = AF_INET;
}

TCPClient::TCPClient(u_short port, const std::string& ip_address) : Socket(SocketType::TYPE_STREAM) {
    set_address(ip_address);
    set_port(port);
}

void TCPClient::make_connection() {
    if (connect(m_socket, reinterpret_cast<sockaddr*>(&m_addr), sizeof(m_addr)) < 0) {
        throw connection_err();
    }
}

void Socket::send(const std::string& message) {
    if (::send(m_socket, message.c_str(), message.length(), 0) < 0) {
        throw send_err();
    }
}

std::string Socket::recv() {
    char server_reply[20000];
    int result = ::recv(m_socket, server_reply, sizeof(server_reply), 0);
    if (result == SOCKET_ERROR || result == 0) {
        throw recv_err();
    }
    return std::string(server_reply, result);
}

TCPServer::TCPServer(u_short port, const std::string& ip_address) : m_listen(Socket::SocketType::TYPE_STREAM) {
    m_listen.set_port(port);
    m_listen.set_address(ip_address);
};

void Socket::bind() {
    if (::bind(m_socket, reinterpret_cast<sockaddr*>(&m_addr), sizeof(m_addr)) == SOCKET_ERROR) {
        throw bind_err();
    }
    listen(m_socket, 3);
}

Socket Socket::accept() {
    socklen_t client_size = sizeof(sockaddr_in);
    sockaddr_in client;
    SOCKET new_socket;
    new_socket = ::accept(m_socket, reinterpret_cast<sockaddr*>(&client), &client_size);
    if (new_socket == INVALID_SOCKET) {
        throw accept_err();
    }
    return Socket(new_socket, client);
}
