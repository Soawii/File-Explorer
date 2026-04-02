#include "EventBus.hpp"

std::mutex EventBus::mtx;
std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> EventBus::eventToCallbacks;

void EventBus::publish(const std::string& event, const std::string& data) {
    std::vector<std::function<void(const std::string&)>> callbacks;
    {
        std::lock_guard<std::mutex> lock(mtx);
        const auto& it = eventToCallbacks.find(event);
        if (it == eventToCallbacks.end()) 
            return;
        callbacks = it->second;
    }
    for (const auto& callback : callbacks) {
        try {
            callback(data);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception when triggering a callback with event " + event + ": " + e.what() << '\n';
        }
    }
}

void EventBus::subscribe(const std::string& event, const std::function<void(const std::string&)>& callback) {
    std::lock_guard<std::mutex> lock(mtx);
    eventToCallbacks[event].push_back(callback);
}