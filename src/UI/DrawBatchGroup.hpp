#pragma once
#include <SFML/Graphics.hpp>

// WIP FOR LATER, DRAW BATCHING, NOT NEEDED YET AND KINDA DIFFICULT TO IMPLEMENT

struct DrawBatch {
    sf::PrimitiveType type;
    int z_index = 0;
    const sf::Texture* texture = nullptr;
    sf::VertexArray vertices;
};

// Batches grouped by viewport so that elements with 'overflow: hidden' can work.
class DrawBatchGroup {
public:
    DrawBatchGroup(bool isHidden) : m_isHidden(isHidden) {}
    DrawBatchGroup(bool isHidden, const sf::IntRect viewport) : m_isHidden(isHidden), m_viewport(viewport) {}

    void beginFrame() {
        for (DrawBatch* b : m_batches) {
            delete b;
        }
        m_batches.clear();
        for (int i = 0; i < m_children.size(); i++) {
            m_children[i]->beginFrame();
            delete m_children[i];
        }
        m_children.clear();
    }

    void submit(const sf::VertexArray& vertices, const sf::Transform& transform, int z_index, const sf::Texture* texture = nullptr) {
        if (vertices.getVertexCount() == 0)
            return;
        DrawBatch* batch = findOrCreateBatch(vertices.getPrimitiveType(), z_index, texture);
        const size_t N = vertices.getVertexCount();

        for (size_t i = 0; i < N; i++) {
            sf::Vertex v = vertices[i];
            v.position = transform.transformPoint(v.position);
            batch->vertices.append(v);
        }
    }

    DrawBatchGroup* createChildViewport(const sf::IntRect& viewport) {
        m_children.push_back(new DrawBatchGroup(true, viewport));
        return m_children.back();
    }

    void draw(sf::RenderTarget& target) {
        int l = 0, r = 0;
        while (l < m_batches.size() && r < m_children.size()) {
            if (m_batches[l]->z_index < m_children[r]->m_batches[0]->z_index) {
                target.draw(m_batches[l]->vertices, sf::RenderStates(m_batches[l]->texture));
                l++;
            }
            else {
                m_children[r]->draw(target);
                r++;
            }
        }
        while (l < m_batches.size()) {
            target.draw(m_batches[l]->vertices, sf::RenderStates(m_batches[l]->texture));
            l++;
        }
        while (r < m_children.size()) {
            m_children[r]->draw(target);
            r++;
        }
    }

    sf::IntRect m_viewport{0,0,0,0};
    std::vector<DrawBatch*> m_batches;
    std::vector<DrawBatchGroup*> m_children;
protected:
    DrawBatch* findOrCreateBatch(sf::PrimitiveType type, int z_index, const sf::Texture* texture) {
        for (auto& b : m_batches) {
            if (b->type == type && b->z_index == z_index && b->texture == texture)
                return b;
        }
        m_batches.push_back(new DrawBatch{type, z_index, texture, sf::VertexArray(type)});
        return m_batches.back();
    }
    const bool m_isHidden = false;
};