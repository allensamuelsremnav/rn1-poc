#include "simple_cpp_sockets.h"
#include <SDL2/SDL.h>

bool running = true;

float steering = 0;
float delta = .01; // Radians

void update_steering(TCPClient& client) {
    client.send("steering " + std::to_string(steering) + "\n");
}


void process_data(Socket& snd) {
    while (running) {
        std::cout << "Sending xyz\n";
        snd.send("XYZ\n");
        sleep(1);
    }
}

void listener(const std::string& cmd) {
    std::cout << "Received echo back of : " << cmd << "\n";
}

TCPServer listenSocket(0);

void listener_thread(u_short port) {
    listenSocket = TCPServer(port+1);
    listenSocket.bind();
    while (running) {
        std::cerr << "Waiting for connection on port " << port+1 << "\n";
        Socket r = listenSocket.accept();
        std::cerr << "Got connection from " << r.getpeername() << ":" << r.getpeerport() << "\n";
        try {
            while (running) {
                listener(r.recv());
            }
        } catch(...) {
        }
    }
}

int main(int argc, char **argv) {
    std::string ip;
    int port = 6379;

    if (argc < 1) {
        std::cerr << "syntax: ip-addr [port]\nport defaults to 6379\n";
        return 1;
    }
    if (argc > 1) {
        ip = std::string(argv[1]);
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }
    std::cerr << "Connecting to " << ip << ":" << port << "\n";
    std::thread(listener_thread, port).detach();
    while (running) {
        TCPClient snd(port, ip);
        try {
            snd.make_connection();
            std::cerr << "Connected to " << snd.getpeername() << ":" << snd.getpeerport() << ", waiting for callback\n";
            process_data(snd);
        } catch(connection_err) {
            sleep(1);
        } catch (send_err) {
            std::cerr << "Disconnected\n";
        }
    }
    return 0;
}
