#include <SFML/Graphics.hpp>
#include "util.hpp"
#include <chrono>
#include "../conf.hpp"

namespace util {
    float getSquaredLengthBetweenPoints(sf::Vector2f pos1, sf::Vector2f pos2) {
        const sf::Vector2f diff = pos2 - pos1;
        return diff.x * diff.x + diff.y * diff.y;
    }

    float getLen(sf::Vector2f vec) {
        return sqrt(vec.x * vec.x + vec.y * vec.y);
    }

    float timeFunction(const std::function<void(void)>& func) {
        auto start = std::chrono::steady_clock::now();
        func();
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<float, std::micro>(end - start).count();
    }

    sf::Transform interpolateTransform(const sf::Transform& prev, const sf::Transform& curr, float alpha) {
        const float* pPrev = prev.getMatrix();
        const float* pCurr = curr.getMatrix();

        sf::Transform result;
        float m[16];
        for (int i = 0; i < 16; ++i) {
            m[i] = pPrev[i] + (pCurr[i] - pPrev[i]) * alpha;
        }
        result = sf::Transform(
            m[0], m[4], m[12],
            m[1], m[5], m[13],
            0.f, 0.f, 1.f
        );
        return result;
    }

    sf::Vector2f normalize(sf::Vector2f v) {
        return v / getLen(v);
    }

    void appendRoundedShadow(sf::VertexArray& verts,
                         sf::FloatRect rect,
                         float radius,
                         float blurRadius,
                         sf::Color color,
                         unsigned int cornerSteps)
    {

        auto addQuad = [&](sf::Vector2f a, sf::Vector2f b,
                        sf::Vector2f c, sf::Vector2f d,
                        sf::Color colA, sf::Color colB)
        {
            verts.append({a, colA});
            verts.append({b, colA});
            verts.append({c, colB});
            verts.append({d, colB});
        };

        const float innerLeft   = rect.left;
        const float innerTop    = rect.top;
        const float innerRight  = rect.left + rect.width;
        const float innerBottom = rect.top + rect.height;

        const float outerLeft   = innerLeft   - blurRadius;
        const float outerTop    = innerTop    - blurRadius;
        const float outerRight  = innerRight  + blurRadius;
        const float outerBottom = innerBottom + blurRadius;

        const float innerR = radius;
        const float outerR = radius + blurRadius;

        sf::Color innerColor = color;  // full alpha
        sf::Color outerColor = color;
        outerColor.a = 0;              // transparent outer edge

        // For each corner, build a triangle strip between inner and outer arcs
        auto appendCorner = [&](float cx, float cy, float startAngle, float endAngle, int signX, int signY)
        {
            for (unsigned int i = 0; i <= cornerSteps; ++i)
            {
                float t = startAngle + (endAngle - startAngle) * (float(i) / cornerSteps);
                float cosT = std::cos(t);
                float sinT = std::sin(t);

                sf::Vector2f innerP = { cx + cosT * innerR * signX,
                                        cy + sinT * innerR * signY };
                sf::Vector2f outerP = { cx + cosT * outerR * signX,
                                        cy + sinT * outerR * signY };

                verts.append({innerP, innerColor});
                verts.append({outerP, outerColor});
            }
        };

        // Top-left corner (180° → 270°)
        appendCorner(innerLeft + innerR, innerTop + innerR, conf::constants::PI, conf::constants::PI * 1.5f, +1, +1);
        // Top-right (270° → 360°)
        appendCorner(innerRight - innerR, innerTop + innerR, conf::constants::PI * 1.5f, conf::constants::PI * 2.f, -1, +1);
        // Bottom-right (0° → 90°)
        appendCorner(innerRight - innerR, innerBottom - innerR, 0.f, conf::constants::PI * 0.5f, -1, -1);
        // Bottom-left (90° → 180°)
        appendCorner(innerLeft + innerR, innerBottom - innerR, conf::constants::PI * 0.5f, conf::constants::PI, +1, -1);

        // Edges (top, right, bottom, left) — optional; helps fill between corners
        // Each is a simple quad strip from inner to outer edge.
        addQuad({innerLeft + innerR, innerTop},
                {innerRight - innerR, innerTop},
                {outerLeft + outerR, outerTop},
                {outerRight - outerR, outerTop},
                innerColor, outerColor);
        // (repeat for other 3 edges)
    }

    std::string formatBytes(size_t bytes) {
        static const std::vector<std::string> endings = {" KB", " MB", " GB", " TB"};
        int chosen_ending = 0;
        float bytes_copy = bytes / 1024.f;
        while (bytes_copy > 512.0f && chosen_ending < int(endings.size()) - 1) {
            bytes_copy /= 1024.0f;
            chosen_ending = chosen_ending + 1;
        }
        std::string bytes_formatted = std::to_string(roundf(bytes_copy * 100.0f) / 100.0f);
        size_t dot_pos = bytes_formatted.find('.');
        if (dot_pos != std::string::npos) {
            bytes_formatted = bytes_formatted.substr(0, dot_pos + 3);
        }
        while (bytes_formatted.back() == '0') {
            bytes_formatted.pop_back();
        }
        if (bytes_formatted.back() == '.') {
            bytes_formatted.pop_back();
        }
        return bytes_formatted + endings[chosen_ending];
    }

    std::string formatDate(const std::string& date) {
        const int space_pos = date.find(' ');
        return date.substr(0, space_pos);
    }
}