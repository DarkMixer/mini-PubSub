#include "server.h"
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/ioctl.h>
#endif

using namespace  worldwidewhat;

Server::Server(int port){
    setup(port);
}

Server::~Server() {
    shutdown();
}

void Server::shutdown(){
    close(m_masterSocket_fd);
#ifdef _WIN32    
    WSACleanup();
#endif    
}

void Server::init(){
    initSocket();
    bindSocket();
    startListen();
}

void Server::loop(){
    fd_temp = fd_master;
    
    int n_sel = select(max_fd + 1, &fd_temp, NULL, NULL, NULL); // Blocks until activity
    if(n_sel < 0) {
        perror("[SERVER] [ERROR] select() failed");
        shutdown();
    }

    for(int n_fd = 0; n_fd <= max_fd; n_fd++) {
        if(FD_ISSET(n_fd, &fd_temp)) {
            if(m_masterSocket_fd == n_fd) {
                incomming_connection();
            } else {
                receiveInput(n_fd);
            }
        }
    }

}

int Server::transmit(uint16_t fd, const char* data){
    
    int n_res = send(fd, data, strlen(data), 0);

    if(n_res <= 0){
        if(n_res == 0){
            if(_onDisconnectCallback != NULL)
                _onDisconnectCallback((uint16_t)fd);
        } else {
            perror("[SERVER] [ERROR] send() failed");
        }
        close(fd);
        FD_CLR(fd, &fd_master);
        return -1;
    }
    return 0;
}

void Server::set_onConnect(std::function<void(uint16_t)> callback) {
    _onConnectCallback = callback;
}

void Server::set_onDisconnect(std::function<void(uint16_t)> callback) {
    _onDisconnectCallback = callback;
}

void Server::set_onInput(std::function<void(uint16_t, char*)> callback) {
    _onInputCallback = callback;
}

int Server::setup(int port) {
#ifdef _WIN32    
    WSADATA info;
    if (WSAStartup(MAKEWORD(2,0), &info)) {
      throw "Could not start WSA";
    }
#endif    
    m_masterSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_masterSocket_fd < 0) {
        perror("[SERVER] [ERRPR] Socket creation failed");
        return -1;
    }

    FD_ZERO(&fd_master);
    FD_ZERO(&fd_temp);

    memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_addr.s_addr = htons(INADDR_ANY);
    m_serverAddr.sin_port = htons(port);

    bzero(m_inBuffer, SERVER_IN_BUFFER_SIZE);
    return 0;
}

void Server::initSocket(){
    int n_optValue = 1;
    int n_res = setsockopt(m_masterSocket_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &n_optValue, sizeof(int));

    if(n_res < 0) {
        perror("[SERVER] [ERROR] setsockopt() failed");
        shutdown();
    }
}

void Server::bindSocket(){
    int n_res = bind(m_masterSocket_fd, (struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    if(n_res < 0) {
        perror("[SERVER] [ERROR] bind() failed");
        return;    
    }

    FD_SET(m_masterSocket_fd, &fd_master);
    max_fd = m_masterSocket_fd;
}

void Server::startListen(){
    int n_res = listen(m_masterSocket_fd, 3);

    if(n_res < 0){
        perror("[SERVER] [ERROR] listen() failed");
    }
}

void Server::incomming_connection(){
    socklen_t n_addrLen = sizeof(m_clientAddr);
    m_tempSocket_fd = accept(m_masterSocket_fd, (struct sockaddr*)&m_clientAddr, &n_addrLen);

    if(m_tempSocket_fd < 0){
        perror("[SERVER] [ERROR] accept() failed");
        return;
    }

    FD_SET(m_tempSocket_fd, &fd_master);
    if(m_tempSocket_fd > max_fd){
        max_fd = m_tempSocket_fd;
    }
    if(_onConnectCallback != NULL)
        _onConnectCallback(m_tempSocket_fd);
}

void Server::receiveInput(int fd){
    std::vector<char> l_bffr;

    char buf[10];
    int n;
    int flag = 1;

    do {
        n = recv(fd, buf, sizeof(buf),0);
        if (n > 0) {
            int bffr_index = 0;
            for(bffr_index = 0; bffr_index < n; bffr_index++){
                l_bffr.push_back(buf[bffr_index]);
            }
#ifdef _WIN32
            ioctlsocket(fd, FIONREAD, &flag);
#else
            ioctl(fd, FIONREAD, &flag);
#endif            
            
        } else {
            flag = 0;
        }
    } while (flag > 0);    

    if(n <= 0){
        if(n == 0){
            if(_onDisconnectCallback != NULL)
                _onDisconnectCallback((uint16_t)fd);
        }
        close(fd);
        FD_CLR(fd, &fd_master);
        return;
    }
    if(_onInputCallback != NULL && l_bffr.data() != NULL)
        _onInputCallback(fd, l_bffr.data());
    
}




