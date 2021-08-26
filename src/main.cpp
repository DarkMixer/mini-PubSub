#include <iostream>
#include "server/server.h"

#ifdef _WIN32
worldwidewhat::Server _server(9000);
#else
worldwidewhat::Server _server(9001);
#endif

/**
 * @brief On new connection established callback function
  * @param client - ID of new client
 */
void onConnect(uint16_t client){
    std::printf("Client with ID %d connected\n", client);
}

/**
 * @brief On connection closed / disconnected callback function.
  * @param client  - ID of the client connected
 */
void onDisconnect(uint16_t client){
    std::printf("Client with ID %d disconnected\n", client);
}

/**
 * @brief On data received from client callback.
 * @param client - ID of the client
 * @param data  - Data received from client
 */
void onInputReceived(uint16_t client, char* data){
    std::printf("Input from client %u : %s\n", client, data);
    _server.transmit(client, "OK");
}

int main(int argc, char *argv[]) {
    std::cout << "Socket server starting" << std::endl;

    // Attach callbacks
    _server.set_onConnect(onConnect);
    _server.set_onDisconnect(onDisconnect);
    _server.set_onInput(onInputReceived);

    // Initialize the server
    _server.init();    

    while(true) {
        _server.loop(); // Maintain the socket (could be moved to thread)
    }
}
