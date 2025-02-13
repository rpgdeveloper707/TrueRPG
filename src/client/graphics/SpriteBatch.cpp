#include "../../pch.h"
#include "SpriteBatch.h"

#include "Graphics.h"
#include <numeric>

SpriteBatch::SpriteBatch(Shader shader, int maxSprites)
    : m_shader(shader),
      m_maxSprites(maxSprites),
      m_vbo(GL_ARRAY_BUFFER),
      m_ibo(GL_ELEMENT_ARRAY_BUFFER)
{
    const int vertexCount = maxSprites * 4;
    const int indexCount = maxSprites * 6;

    m_vao.bind();
    m_vbo.bind();

    // This is a little trick.
    // Instead of putting the data into vbo, we just allocate memory for later use
    m_vbo.setData(nullptr, sizeof(Vertex) * vertexCount, GL_DYNAMIC_DRAW);

    // Coords
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Texture index
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Pattern:
    // 0, 1, 2, 2, 3, 0
    // 4, 5, 6, 6, 7, 4
    // etc.
    auto *indices = new unsigned int[indexCount];
    unsigned int offset = 0;
    for (int i = 0; i < indexCount; i += 6)
    {
        indices[i + 0] = 0 + offset;
        indices[i + 1] = 1 + offset;
        indices[i + 2] = 2 + offset;

        indices[i + 3] = 2 + offset;
        indices[i + 4] = 3 + offset;
        indices[i + 5] = 0 + offset;

        offset += 4;
    };
    m_ibo.bind();
    m_ibo.setData(indices, indexCount * sizeof(unsigned int), GL_STATIC_DRAW);

    m_vbo.unbind();
    m_vao.unbind();
    delete[] indices;
}

void SpriteBatch::begin()
{
    for (auto &m_layer : m_layers)
    {
        m_layer.clear();
    }
    m_spritesSize = 0;
    m_texturesSize = 0;
}

void SpriteBatch::end()
{
    // In this method we draw all vertices at once with a single draw call
    m_vbo.bind();

    // Insert everything into one vector
    auto *vertices = new Vertex[m_spritesSize * 4];
    int currentVertex = 0;

    for (const auto &layer : m_layers)
    {
        for (const auto &quad : layer)
        {
            vertices[currentVertex].position = quad.vertices[0].position;
            vertices[currentVertex].texCoord = quad.vertices[0].texCoords;
            vertices[currentVertex].color = quad.color;
            vertices[currentVertex].texId = quad.texId;

            vertices[currentVertex + 1].position = quad.vertices[1].position;
            vertices[currentVertex + 1].texCoord = quad.vertices[1].texCoords;
            vertices[currentVertex + 1].color = quad.color;
            vertices[currentVertex + 1].texId = quad.texId;

            vertices[currentVertex + 2].position = quad.vertices[2].position;
            vertices[currentVertex + 2].texCoord = quad.vertices[2].texCoords;
            vertices[currentVertex + 2].color = quad.color;
            vertices[currentVertex + 2].texId = quad.texId;

            vertices[currentVertex + 3].position = quad.vertices[3].position;
            vertices[currentVertex + 3].texCoord = quad.vertices[3].texCoords;
            vertices[currentVertex + 3].color = quad.color;
            vertices[currentVertex + 3].texId = quad.texId;
            currentVertex += 4;
        }
    }

    // Put our vertices into the allocated memory
    m_vbo.setSubData(vertices, 0, currentVertex * sizeof(Vertex));

    m_shader.use();

    int *ids = new int[m_texturesSize];
    std::iota(ids, ids + m_texturesSize, 0);
    m_shader.setUniform("textures", m_texturesSize, ids);

    m_shader.setUniform("model", glm::mat4(1));

    for (int i = 0; i < m_texturesSize; i++)
    {
        m_textures[i].bind(i);
    }

    m_vao.bind();
    glDrawElements(GL_TRIANGLES, m_spritesSize * 6, GL_UNSIGNED_INT, nullptr);

    delete[] vertices;
    delete[] ids;
}

// This function makes our rect a bit smaller.
// It helps to prevent strange artifacts with textures.
static FloatRect prepareRect(IntRect rect)
{
    float offset = 0.5f;

    auto left = (float)rect.getLeft();
    auto bottom = (float)rect.getBottom();
    auto width = (float)rect.getWidth();
    auto height = (float)rect.getHeight();

    left += glm::sign(width) * offset;
    bottom += glm::sign(height) * offset;

    width = glm::sign(width) * (std::abs(width) - 2 * offset);
    height = glm::sign(height) * (std::abs(height) - 2 * offset);

    return {left, bottom, width, height};
}

static glm::vec2 toTexCoords(Texture &texture, float x, float y)
{
    return {(float)x / texture.getWidth(), (float)y / texture.getHeight()};
}

void SpriteBatch::draw(const Sprite &sprite, int layer, int order)
{
    if (layer >= MaxLayers)
    {
        std::cerr << "Cannot draw a sprite! The maximum number of layers is " << MaxLayers << std::endl;
        return;
    }

    // Actually we draw nothing here. In this method we just collect the sprites to draw them later
    if (m_spritesSize >= m_maxSprites)
    {
        std::cerr << "Cannot draw a sprite! Maximum number of sprites reached!" << std::endl;
        return;
    }
    m_spritesSize++;

    glm::vec2 quadPos = sprite.getPosition() - sprite.getOrigin() * sprite.getScale();
    IntRect rect = sprite.getTextureRect();

    Texture texture = sprite.getTexture();

    int i;
    for (i = 0; i < m_texturesSize; i++)
    {
        if (m_textures[i].getId() == texture.getId())
        {
            break;
        }
    }

    if (i >= MaxTextures)
    {
        std::cerr << "Cannot draw a sprite with texture " << texture.getPath() << "! Maximum number of m_texturesSize reached!"
                  << std::endl;
        return;
    }

    // If we get to the end, add a new texture
    if (i == m_texturesSize)
    {
        m_textures[i] = texture;
        m_texturesSize++;
    }

    auto texId = static_cast<float>(i);

    float w = (float)std::abs(rect.getWidth()) * sprite.getScale().x;
    float h = (float)std::abs(rect.getHeight()) * sprite.getScale().y;

    // Create a layer if absent
    auto &set = m_layers[layer];

    FloatRect r = prepareRect(rect);

    set.insert({{{quadPos, // bottom left
                     toTexCoords(texture, r.getLeft(), r.getBottom())},
                    {quadPos + glm::vec2(w, 0.f), // bottom right
                        toTexCoords(texture, r.getLeft() + r.getWidth(), r.getBottom())},
                    {quadPos + glm::vec2(w, h), // top right
                        toTexCoords(texture, r.getLeft() + r.getWidth(), r.getBottom() + r.getHeight())},
                    {quadPos + glm::vec2(0.f, h), // top left
                        toTexCoords(texture, r.getLeft(), r.getBottom() + r.getHeight())}},
        sprite.getColor(), texId, order});
}

void SpriteBatch::setShader(Shader shader)
{
    m_shader = shader;
}

glm::mat4 SpriteBatch::getProjectionMatrix()
{
    return m_projMat;
}

void SpriteBatch::setProjectionMatrix(glm::mat4 projMat)
{
    m_projMat = projMat;
    m_shader.use();
    m_shader.setUniform("projection", projMat);
}

glm::mat4 SpriteBatch::getViewMatrix()
{
    return m_viewMat;
}

void SpriteBatch::setViewMatrix(glm::mat4 viewMat)
{
    m_viewMat = viewMat;
    m_shader.use();
    m_shader.setUniform("view", viewMat);
}
void SpriteBatch::destroy()
{
    m_vao.destroy();
    m_vbo.destroy();
    m_ibo.destroy();
}
