#ifndef _SERVER_H_
#define _SERVER_H_

#include <iostream>
#include <functional>
#include <unistd.h>

#ifdef _WIN32 // Include files for windows system

#include <winsock2.h>
#include <stdio.h>

#define INET6_ADDRSTRLEN 46
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
typedef int socklen_t;

#else // Include files for linux system

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h> //sockaddr, socklen_t
#include <arpa/inet.h>
#include <netdb.h>

#endif

namespace worldwidewhat {
    #define SERVER_IN_BUFFER_SIZE 2048  // Incomming data buffer length
    /**
     * @brief TCP/IP Server
     * @author lh@ctcoin.com
     * @version 1.0
     */
    class Server {
        public:
        Server(int port=9000);
        virtual ~Server();

        /** @brief Shutdown socket and server */
        void shutdown();
        /** @brief Initialize server and start listen */
        void init();
        /** @brief Maintain server connections */
        void loop();

        /**
         * @brief Transmit data to a client
         * 
         * @param fd Client ID
         * @param data Data to transmit
         * @return int Success
         */
        int transmit(uint16_t fd, const char* data);

        /**
         * @brief Set the onConnect object
         * @param callback pointer to callback function
         */
        void set_onConnect(std::function<void(uint16_t)> callback);
        /**
         * @brief Set the onDisconnect object
         * @param callback pointer to callback function
         */
        void set_onDisconnect(std::function<void(uint16_t)> callback);

        /**
         * @brief Set the onInput object
         * @param callback pointer to callback function
         */
        void set_onInput(std::function<void(uint16_t, char*)> callback);

        private: // Functions
        /** @brief Setup socket */
        int setup(int port);
        /** @brief Initialize socket */
        void initSocket();
        /** @brief Bind socket */
        void bindSocket();
        /** @brief Start listen on socket */
        void startListen();
        /** @brief Handle new incomming connection */
        void incomming_connection();
        /**
         * @brief Receive incomming data
         * @param fd Client ID
         */
        void receiveInput(int fd);

        private: // Variables
        
        fd_set _master_fd;
        fd_set _temp_fd;

        uint16_t _max_fd;

        int _masterSocket_fd;
        int _tempSocket_fd;

        struct sockaddr_storage _clientAddr;
        struct sockaddr_in _serverAddr;

        char _inBuffer[SERVER_IN_BUFFER_SIZE];
//        char _removeIP[INET6_ADDRSTRLEN];

        std::function<void(uint16_t)> _onConnectCallback;
        std::function<void(uint16_t)> _onDisconnectCallback;
        std::function<void(uint16_t, char*)> _onInputCallback;
    };
}

#endif //_SERVER_H_