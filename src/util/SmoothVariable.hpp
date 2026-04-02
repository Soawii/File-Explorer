#pragma once

template <typename T>
class SmoothVariable {
public:
    inline const static float NO_SMOOTHING = -1.0f;

    SmoothVariable(T startingVar, float smoothing) {
        m_smoothing = smoothing;
        m_var = startingVar;
        m_smoothedVar = startingVar;
    }
    SmoothVariable operator=(T other) {
        if (m_smoothing == NO_SMOOTHING) {
            m_smoothedVar = other;
        }
        m_var = other;
        return *this;
    }
    void setInstantly(T var) {
        m_var = var;
        m_smoothedVar = var;
    }
    void setSmoothing(float smoothing) {
        m_smoothing = smoothing;
    }
    T get() {
        return m_smoothedVar;
    }
    T getActual() {
        return m_var;
    }
    void update(float dt) {
        m_smoothedVar += (m_var - m_smoothedVar) * m_smoothing * dt;
    }
private:
    T m_var, m_smoothedVar;
    float m_smoothing;
};