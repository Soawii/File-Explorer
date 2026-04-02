#pragma once
#include <vector>
#include <functional>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "../util/TimingVariable.hpp"

namespace ui {

enum class UIAnimationDirection {
    FORWARD,
    BACKWARD
};

enum class UIAnimationPlayMode {
    ONCE,
    LOOP,
    PING_PONG
};

template<typename T>
struct UIKeyframe {
    float time; // [0-1]
    T value;
    float (*easing)(float) = TimingFunctions::linear;
};

class IUIAnimation {
public:
    std::string m_name;
    UIAnimationPlayMode m_mode = UIAnimationPlayMode::ONCE;
    UIAnimationDirection m_direction = UIAnimationDirection::FORWARD;
    float m_duration = 0.0f;
    bool m_playing = false;
    float m_pausedAt = 0.0f;
    std::chrono::steady_clock::time_point m_startTime;

    IUIAnimation(std::string name, float duration, UIAnimationPlayMode mode = UIAnimationPlayMode::ONCE) 
    : m_name(name), m_duration(duration), m_mode(mode) {}

    virtual void update() = 0;

    void play() {
        m_playing = true;
        m_direction = UIAnimationDirection::FORWARD;
        m_startTime = conf::time::now;
    }

    void stop() {
        m_playing = false;
    }

    void pause() {
        m_playing = false;
        std::chrono::duration<float> d(conf::time::now - m_startTime);
        m_pausedAt = d.count();
    }

    void resume() {
        m_playing = true;
        m_startTime = conf::time::now - std::chrono::milliseconds(int(m_pausedAt * 1000));
    }

    void changeDirection() {
        m_direction = m_direction == UIAnimationDirection::FORWARD ? UIAnimationDirection::BACKWARD : UIAnimationDirection::FORWARD;
        const float time_passed = std::chrono::duration<float>(conf::time::now - m_startTime).count();
        m_startTime = conf::time::now - std::chrono::milliseconds(int(m_duration * 1000)) + std::chrono::milliseconds(int(time_passed * 1000));
    }
};

template<typename T>
class UIAnimation : public IUIAnimation {
public:
    std::vector<UIKeyframe<T>> m_keyframes;
    std::function<void(const T&)> m_setter;

    UIAnimation() = default;
    UIAnimation(std::string name, std::vector<UIKeyframe<T>> keyframes, float duration, std::function<void(const T&)> setter, UIAnimationPlayMode mode = UIAnimationPlayMode::ONCE)
        : IUIAnimation(name, duration, mode), m_keyframes(keyframes), m_setter(setter) {}

    void update() override {
        if (!m_playing || m_keyframes.size() < 2) {
            return;
        }

        const std::chrono::duration<float> dur(conf::time::now - m_startTime);
        float t = dur.count() / m_duration;

        if (t > 1.0f) {
            switch (m_mode) {
                case UIAnimationPlayMode::ONCE:
                    m_playing = false;
                    m_setter(m_keyframes.back().value);
                    return;
                case UIAnimationPlayMode::LOOP:
                    m_startTime = conf::time::now;
                    t--;
                    break;
                case UIAnimationPlayMode::PING_PONG:
                    m_direction = (m_direction == UIAnimationDirection::FORWARD) ? UIAnimationDirection::BACKWARD : UIAnimationDirection::FORWARD;
                    m_startTime = conf::time::now;
                    t--;
                    break;
            }
        }

        if (m_direction == UIAnimationDirection::BACKWARD) {
            t = 1.0f - t;
        }

        for (size_t i = 1; i < m_keyframes.size(); ++i) {
            if (t <= m_keyframes[i].time) {
                const UIKeyframe<T>& k0 = m_keyframes[i - 1];
                const UIKeyframe<T>& k1 = m_keyframes[i];
                float localT = (t - k0.time) / (k1.time - k0.time);
                float eased = k1.easing(localT);
                m_setter(k0.value + (k1.value - k0.value) * eased);
                return;
            }
        }
        m_setter(m_keyframes.back().value);
        return;
    }
};

}