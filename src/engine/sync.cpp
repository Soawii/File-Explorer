#include "sync.hpp"

namespace sync {
    void init() {}

    // STATE
    std::mutex StateManager::mtx;
	State StateManager::prev_state = State::NORMAL;
    State StateManager::curr_state = State::NORMAL;
    std::array<std::vector<std::function<void(void)>>, size_t(State::STATE_COUNT)> StateManager::stayCallbacks;
    std::array<std::vector<std::function<void(void)>>, size_t(State::STATE_COUNT)> StateManager::enterCallbacks;
    std::array<std::vector<std::function<void(void)>>, size_t(State::STATE_COUNT)> StateManager::leaveCallbacks;

    void StateManager::setState(State newState) {
        std::lock_guard<std::mutex> lock(mtx);
        curr_state = newState;
	}
    State StateManager::getState() {
        std::lock_guard<std::mutex> lock(mtx);
        return curr_state;
	}
    void StateManager::triggerCallbacks() {
        triggerChangeCallbacks();
        prev_state = curr_state;
        triggerStayCallbacks();
    }
	void StateManager::triggerChangeCallbacks() {
        if (curr_state == prev_state)
            return; 
        for (size_t i = 0; i < leaveCallbacks[size_t(prev_state)].size(); i++) {
            leaveCallbacks[size_t(prev_state)][i]();
        }
        for (size_t i = 0; i < enterCallbacks[size_t(curr_state)].size(); i++) {
            enterCallbacks[size_t(curr_state)][i]();
        }
	}
    void StateManager::triggerStayCallbacks() {
        for (size_t i = 0; i < stayCallbacks[size_t(curr_state)].size(); i++) {
            stayCallbacks[size_t(curr_state)][i]();
        }
	}
    void StateManager::addCallback(State state, const std::function<void(void)>& callback) {
        stayCallbacks[size_t(state)].push_back(callback);
	}
	void StateManager::addEnterCallback(State state, const std::function<void(void)>& callback) {
        enterCallbacks[size_t(state)].push_back(callback);
	}
    void StateManager::addLeaveCallback(State state, const std::function<void(void)>& callback) {
        leaveCallbacks[size_t(state)].push_back(callback);
	}

    // TASKS
    std::mutex TaskManager::mtx;
    std::queue<std::pair<std::chrono::steady_clock::time_point, std::function<void(void)>>> TaskManager::tasks;

    void TaskManager::addTask(std::chrono::steady_clock::time_point time_point, const std::function<void(void)>& task) {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push(std::make_pair(time_point, task));
    }
    void TaskManager::consumeTasks(std::chrono::steady_clock::time_point time_point) {
        while (!tasks.empty()) {
            auto top = tasks.front();
            if (top.first > time_point)
                break;
            tasks.pop();
            top.second();
        }
    }
}