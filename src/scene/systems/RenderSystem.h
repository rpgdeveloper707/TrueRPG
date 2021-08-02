#ifndef RPG_RENDERSYSTEM_H
#define RPG_RENDERSYSTEM_H

#include <entt.hpp>
#include "../../graphics/SpriteBatch.h"
#include "../components/TransformComponent.h"

class RenderSystem
{
    entt::registry& m_registry;
    Shader m_shader;
    SpriteBatch m_batch;
public:
    RenderSystem(entt::registry& registry);

    void draw();

    void destroy();

private:
    TransformComponent computeTransform(entt::entity entity);
};

#endif //RPG_RENDERSYSTEM_H