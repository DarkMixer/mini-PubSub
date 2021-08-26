#include "server.h"

using namespace  worldwidewhat;

Server::Server(int port){
    setup(port);
}

Server::~Server() {
    shutdown();
}

void Server::shutdown(){
    close(_masterSocket_fd);
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
    _temp_fd = _master_fd;
    
    int n_sel = select(_max_fd + 1, &_temp_fd, NULL, NULL, NULL); // Blocks until activity
    if(n_sel < 0) {
        perror("[SERVER] [ERROR] select() failed");
        shutdown();
    }

    for(int n_fd = 0; n_fd <= _max_fd; n_fd++) {
        if(FD_ISSET(n_fd, &_temp_fd)) {
            if(_masterSocket_fd == n_fd) {
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
            _onDisconnectCallback((uint16_t)fd);
        } else {
            perror("[SERVER] [ERROR] send() failed");
        }
        close(fd);
        FD_CLR(fd, &_master_fd);
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
    _masterSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_masterSocket_fd < 0) {
        perror("[SERVER] [ERRPR] Socket creation failed");
        return -1;
    }

    FD_ZERO(&_master_fd);
    FD_ZERO(&_temp_fd);

    memset(&_serverAddr, 0, sizeof(_serverAddr));
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_addr.s_addr = htons(INADDR_ANY);
    _serverAddr.sin_port = htons(port);

    bzero(_inBuffer, SERVER_IN_BUFFER_SIZE);
    return 0;
}

void Server::initSocket(){
    int n_optValue = 1;
    int n_res = setsockopt(_masterSocket_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &n_optValue, sizeof(int));

    if(n_res < 0) {
        perror("[SERVER] [ERROR] setsockopt() failed");
        shutdown();
    }
}

void Server::bindSocket(){
    int n_res = bind(_masterSocket_fd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr));
    if(n_res < 0) {
        perror("[SERVER] [ERROR] bind() failed");
        return;    
    }

    FD_SET(_masterSocket_fd, &_master_fd);
    _max_fd = _masterSocket_fd;
}

void Server::startListen(){
    int n_res = listen(_masterSocket_fd, 3);

    if(n_res < 0){
        perror("[SERVER] [ERROR] listen() failed");
    }
}

void Server::incomming_connection(){
    socklen_t n_addrLen = sizeof(_clientAddr);
    _tempSocket_fd = accept(_masterSocket_fd, (struct sockaddr*)&_clientAddr, &n_addrLen);

    if(_tempSocket_fd < 0){
        perror("[SERVER] [ERROR] accept() failed");
        return;
    }

    FD_SET(_tempSocket_fd, &_master_fd);
    if(_tempSocket_fd > _max_fd){
        _max_fd = _tempSocket_fd;
    }

    _onConnectCallback(_tempSocket_fd);
}

void Server::receiveInput(int fd){
    int n_recv_size = recv(fd, _inBuffer, SERVER_IN_BUFFER_SIZE, 0);

    if(n_recv_size <= 0){
        if(n_recv_size == 0){
            _onDisconnectCallback((uint16_t)fd);
        } else {
            perror("[SERVER] [ERROR] recv() failed");
        }
        close(fd);
        FD_CLR(fd, &_master_fd);
    } else {
        _onInputCallback(fd, _inBuffer);
    }
    
    bzero(&_inBuffer, SERVER_IN_BUFFER_SIZE);
}




