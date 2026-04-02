#pragma once
#include <vector>
#include <functional>
#include <mutex>
#include <array>
#include <queue>
#include <thread>

namespace sync {
    void init();

    enum class State {
        NORMAL = 0,
        STATE_COUNT
    };

    class StateManager {
    private:
        static std::mutex   mtx;
        static State        prev_state, curr_state;
        static std::array<std::vector<std::function<void(void)>>, size_t(State::STATE_COUNT)> enterCallbacks;
        static std::array<std::vector<std::function<void(void)>>, size_t(State::STATE_COUNT)> leaveCallbacks;
        static std::array<std::vector<std::function<void(void)>>, size_t(State::STATE_COUNT)> stayCallbacks;
    public:
        static void setState(State newState);
        static State getState();
        static void triggerCallbacks();
        static void triggerChangeCallbacks();
        static void triggerStayCallbacks();
        static void addCallback(State state, const std::function<void(void)>& callback);
        static void addEnterCallback(State state, const std::function<void(void)>& callback);
        static void addLeaveCallback(State state, const std::function<void(void)>& callback);
    };

    class TaskManager {
    private:
        static std::mutex mtx;
        static std::queue<std::pair<std::chrono::steady_clock::time_point, std::function<void(void)>>> tasks;
    public:
        static void addTask(std::chrono::steady_clock::time_point time_point, const std::function<void(void)>& task); 
        static void consumeTasks(std::chrono::steady_clock::time_point time_point); 
    };
}