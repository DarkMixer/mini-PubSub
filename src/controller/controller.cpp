#include <bits/stdc++.h>

#include "controller.h"

using namespace worldwidewhat;

void Controller::setTransmitter(std::function<void(uint16_t, std::string, std::string)> transmitter){
    m_transmitter = transmitter;
}

void Controller::addSubscription(uint16_t client, std::string subscription){
    std::map<std::string, std::vector<uint16_t>>::iterator it = m_subscriper.find(subscription);

    if(it != m_subscriper.end()) {
        it->second.push_back(client);
    } else {
        m_subscriper.insert(std::make_pair(subscription, (std::vector<uint16_t>){client}));
    }

    std::map<std::string, std::string>::iterator it_pub = m_publish.find(subscription);
    if(it_pub != m_publish.end()){
        if(m_transmitter != NULL) m_transmitter(client, it_pub->first, it_pub->second);        
    }

}

void Controller::removeSubscription(uint16_t client, std::string subscription){
    std::map<std::string, std::vector<uint16_t>>::iterator it = m_subscriper.find(subscription);
    if(it != m_subscriper.end()) {
        it->second.erase(std::remove(it->second.begin(), it->second.end(), client), it->second.end());        
    }
}

void Controller::removeClientSubscription(uint16_t client){
    std::map<std::string, std::vector<uint16_t>>::iterator it;
    for(it = m_subscriper.begin(); it != m_subscriper.end(); it++){
        removeSubscription(client, it->first);
    }
}

void Controller::publishItem(std::string key, std::string value){

    std::map<std::string, std::string>::iterator it = m_publish.find(key);
    if(it != m_publish.end()) {
        it->second = value;
    } else {
        m_publish.insert(std::make_pair(key, value));
    }

    pushToSubscripers(key, value);
}

void Controller::pushToSubscripers(std::string key, std::string value){
    std::map<std::string, std::vector<uint16_t>>::iterator it = m_subscriper.find(key);
    if(it != m_subscriper.end()){
        for(uint16_t n_client : it->second){
            if(m_transmitter != NULL) m_transmitter(n_client, key, value);
        }
    }
}

