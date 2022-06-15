#include <iostream>
#include <fstream>
#include <string.h>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>

#elif __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif
using namespace std;

class SSocket {
    int af;
    struct recvdata {
        char* value;
        unsigned long long length;
    };
    struct sockaddr_in sock, client;
public:
#ifdef _WIN32
    WSADATA wsa;
    SOCKET s;
#elif __linux__
    int s;
#endif

    SSocket(int _af, int type) {
        af = _af;
#ifdef _WIN32
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
        if ((s = socket(af, type, 0)) == INVALID_SOCKET)
        {
            perror("Could not create socket");
            exit(EXIT_FAILURE);
        }



    }
#ifdef _WIN32
    SSocket(SOCKET ss) {
        s = ss;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }
#elif __linux__
    SSocket(int ss) {
        s = ss;
    }
#endif


    void sconnect(string ip, int port) {
        if (ip != "") {
            if (ip == "localhost") {
                ip = "127.0.0.1";
            }
            sock.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        else {
            sock.sin_addr.s_addr = INADDR_ANY;
        }
        sock.sin_family = af;
        sock.sin_port = htons(port);

        if (connect(s, (struct sockaddr*)&sock, sizeof(sock)) == SOCKET_ERROR) {
            perror("Connection error");
            exit(EXIT_FAILURE);
        }
    }

    void sbind(string ip, int port) {
        if (ip != "") {
            if (ip == "localhost") {
                ip = "127.0.0.1";
            }
            sock.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        else {
            sock.sin_addr.s_addr = INADDR_ANY;
        }
        sock.sin_family = af;
        sock.sin_port = htons(port);

        if (bind(s, (struct sockaddr*)&sock, sizeof(sock)) == SOCKET_ERROR) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

    }

    void slisten(int clients) {
        if (listen(s, clients) == SOCKET_ERROR) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
    }

    SSocket saccept() {
        int c = sizeof(struct sockaddr_in);
#ifdef _WIN32
        auto new_socket = accept(s, (struct sockaddr*)&client, &c);
#elif __linux__
        auto new_socket = accept(s, (struct sockaddr*)&client, (socklen_t*)&c);
#endif
        if (new_socket == INVALID_SOCKET) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        return SSocket(new_socket);
    }

    void ssend(string data) {
#ifdef _WIN32

        send(s, data.c_str(), data.length(), 0);
#elif __linux__
        send(s, data.c_str(), data.length(), MSG_NOSIGNAL);
#endif
    }

    void ssend(const char * data) {
#ifdef _WIN32
        send(s, data, strlen(data), 0);
#elif __linux__
        send(s, data, strlen(data), MSG_NOSIGNAL);
#endif
    }

    void ssend_file(ifstream& file) {
        char buffer[65536];
        while (file.tellg() != -1) {
            file.read(buffer, 65536);
#ifdef _WIN32
        send(s, buffer, file.gcount(), 0);
#elif __linux__
        send(s, buffer, file.gcount(), MSG_NOSIGNAL);
#endif
        memset(buffer, 0, sizeof(buffer));
        }
    }

    string srecv(int length) {
        if (length > 32768){
            cout << "Error: srecv max value 32768" << endl;
            exit(EXIT_FAILURE);
        }
        char buffer[32768];
        recv(s, buffer, length, 0);
        string recv_data = buffer;
        return recv_data;

    }

    recvdata srecv_char(int length) {
        if (length > 32768) {
            cout << "Error: srecv_char max value 32768" << endl;
            exit(EXIT_FAILURE);
        }
        recvdata data;
        char buffer[32768];
        data.length = recv(s, buffer, length, 0);
        data.value = buffer;
        return data;

    }

    void sclose() {
#ifdef _WIN32
        closesocket(s);
        WSACleanup();
#elif __linux__
        close(s);
#endif
    }
};