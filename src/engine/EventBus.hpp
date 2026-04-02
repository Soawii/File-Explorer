#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <iostream>

class EventBus {
private:
    static std::mutex mtx;
    static std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> eventToCallbacks;
public:
    static void publish(const std::string& event, const std::string& data = "");
    static void subscribe(const std::string& event, const std::function<void(const std::string&)>& callback);
};