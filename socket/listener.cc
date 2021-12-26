#include "simple_cpp_sockets.h"
#include <thread>

//
// Forks off a separate server to handle the socket
//

const int port = 6379;

void handle_conn(Socket& s) {
    while (true) {
        std::string cmd = s.recv();
        std::cout << "Got cmd: " << cmd << "\n";
    }
}

void socket_listener() {
    TCPServer server(port);
    try {
        server.bind();
    } catch(bind_err) {
        std::cout << "Can't bind to port " << port << "\n";
        return;
    }
    while (true) {
        std::cerr << "Waiting to connect\n";
        Socket s = server.accept();
        try {
            handle_conn(s);
        } catch(recv_err) {
            std::cerr << "Receive error\n";
        }
    }
}

std::thread listener_thread;

void initialize() {
    listener_thread = std::thread(socket_listener);
}

int main() {
    initialize();
    while (1) sleep(1);
    return 0;
}
