#include <iostream>
#include "server/server.h"
#include "controller/controller.h"
#include "json.hpp"

using json = nlohmann::json;

#ifdef _WIN32
worldwidewhat::Server _server(9000);
#else
worldwidewhat::Server _server(9001);
#endif

worldwidewhat::Controller _controller;


void transmitResponse(uint16_t client, bool OK){
    json j_response ={{"type", "ACK"}, {"value", OK? "OK": "ERR"}};
    _server.transmit(client, j_response.dump().c_str());
}
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
    _controller.removeClientSubscription(client);
}

/**
 * @brief On data received from client callback.
 * @param client - ID of the client
 * @param data  - Data received from client
 */
void onInputReceived(uint16_t client, char* data){
    
    json j_receive = json::parse(data, nullptr, false);

    if (!j_receive.is_discarded()){
        try{
        std::string str_type = j_receive.value("type", "");
        std::string str_topic = j_receive.value("topic", "");
        if(str_type == "SUB"){
            if(str_topic.length() > 0){
                transmitResponse(client, true);
                _controller.addSubscription(client, str_topic);
            }else {
                transmitResponse(client, false);
            }
        } else if (str_type == "PUB"){
            std::string str_value = j_receive.value("value", "");
            if(str_topic.length() > 0 && str_value.length() > 0) {
                transmitResponse(client, true);
                _controller.publishItem(str_topic, str_value);
            } else {
                transmitResponse(client, false);
            }
        } else if (str_type == "ACK"){

        } else {
            std::printf("ERROR 11\n");
            transmitResponse(client, false);
            return;    
        }
        }catch(...){
            transmitResponse(client, false);    
        }
    } else {
        transmitResponse(client, false);
        std::printf("ERROR 12\n");
    }
    j_receive.clear();
}

void transmitToClient(uint16_t client, std::string key, std::string value){
    json j_transmit {{"type", "EVENT"}, {"topic", key}, {"value", value}};
    _server.transmit(client,j_transmit.dump().c_str());
}

int main(int argc, char *argv[]) {
    std::cout << "Socket server starting" << std::endl;

    _controller.setTransmitter(transmitToClient);

    // Attach callbacks
    _server.set_onConnect(onConnect);
    _server.set_onDisconnect(onDisconnect);
    _server.set_onInput(onInputReceived);

    _server.init();

    while(true) {
        _server.loop(); // Maintain the socket (could be moved to thread)
    }
}
