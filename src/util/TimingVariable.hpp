#pragma once
#include <functional>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "../conf.hpp"

namespace TimingFunctions {
    float linear(float t);
    float easeInQuad(float t);
    float easeOutQuad(float t);
    float easeInOutQuad(float t);
    float smoothstep(float t);
    float easeInOutSine(float t);
}

sf::Color operator*(const sf::Color& left, float right);

template <typename T>
class TimingVariable {
public:
    TimingVariable(T startingVar, float duration, float(*timingFunction)(float)) 
    : m_timingFunction(timingFunction), m_start(startingVar), m_end(startingVar) {
        m_duration = duration;
        m_timepoint = std::chrono::steady_clock::now() - std::chrono::seconds(10);
        m_dirty = false;
    }

    TimingVariable operator=(T other) {
        if (m_duration < 0.001f) {
            if (m_end != other) {
                m_dirty = true;
            }
            m_start = other;
            m_end = other;
            return *this;
        }
        if (m_end == other) {
            return *this;
        }
        float timeLeft = 0.0f;
        if (m_start != m_end) {
            if (m_start == other) {
                std::chrono::duration<float> dur(conf::time::now - m_timepoint);
                float secondsPassed = dur.count();
                timeLeft = std::max(0.0f, m_duration - secondsPassed);
                m_start = m_end;
            }
            else {
                m_start = get();
            }
        }
        m_end = other;
        m_timepoint = conf::time::now - std::chrono::milliseconds(int(timeLeft * 1000.0f));
        return *this;
    }
    void setInstantly(T var) {
        if (m_end != var) {
            m_dirty = true;
        }
        m_start = var;
        m_end = var;
    }
    T get() {
        if (m_duration == 0.0f)
            return m_end;
        std::chrono::duration<float> dur(conf::time::now - m_timepoint);
        float secondsPassed = dur.count();

        if (secondsPassed > m_duration) {
            m_start = m_end;
            return m_end;
        }
        return m_start + (m_end - m_start) * (*m_timingFunction)(secondsPassed / m_duration);
    }
    T getActual() {
        std::chrono::duration<float> dur(conf::time::now - m_timepoint);
        float secondsPassed = dur.count();
        if (secondsPassed > m_duration) {
            m_start = m_end;
        }
        return m_end;
    }

    void setDuration(float t) {
        m_duration = t;
    }
    void setTimingFunction(float(*timingFunction)(float)) {
        m_timingFunction = timingFunction;
    }
    void setParameters(float t, float(*timingFunction)(float)) {
        setDuration(t);
        setTimingFunction(timingFunction);
    }
    bool wasValueChanged() {
        if (m_dirty) {
            m_dirty = false;
            return true;
        }
        return m_start != m_end;
    }

    bool m_dirty;
private:
    T m_start, m_end;
    float(*m_timingFunction)(float);
    std::chrono::steady_clock::time_point m_timepoint;
    float m_duration;
};