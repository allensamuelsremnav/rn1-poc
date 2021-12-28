#include "simple_cpp_sockets.h"
#include <thread>

//
// Forks off a separate server to handle the socket
//

const int port = 6379;

Socket *log_socket = nullptr;

void handle_conn(Socket& rcv) {
    while (true) {
        std::string cmd = rcv.recv();
        std::cout << "Got cmd: " << cmd << "\n";
        if (log_socket) {
            std::cout << "Resending cmd: " << cmd << "\n";
            log_socket->send(cmd);
        }
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
        std::cerr << "Waiting to connect on port " << port << "\n";
        Socket rcv = server.accept();
        std::cerr << "Got connection from " << rcv.getpeername() << "\n";
        TCPClient snd(port+1, rcv.getpeername());
        snd.make_connection();
        std::cerr << "Opened reverse connection\n";
        log_socket = &snd;
        try {
            handle_conn(rcv);
        } catch(recv_err) {
            std::cerr << "Receive error\n";
        }
        log_socket = nullptr;
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
