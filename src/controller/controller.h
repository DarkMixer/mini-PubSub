#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_
#include <inttypes.h>
#include <string.h>
#include <map>
#include <vector>
#include <functional>

namespace worldwidewhat
{
    class Controller
    {
    private:
        std::map<std::string, std::string> m_publish;
        std::map<std::string, std::vector<uint16_t>> m_subscriper;        
        std::function<void(uint16_t, std::string, std::string)> m_transmitter;
    public:
        Controller(){}
        ~Controller(){}

        void setTransmitter(std::function<void(uint16_t, std::string, std::string)> transmitter);

        void addSubscription(uint16_t client, std::string subscription);
        void removeSubscription(uint16_t client, std::string subscription);

        void removeClientSubscription(uint16_t client);

        void publishItem(std::string key, std::string value);

        private:
        void pushToSubscripers(std::string key, std::string value);
    };
}
#endif // _CONTROLLER_H_