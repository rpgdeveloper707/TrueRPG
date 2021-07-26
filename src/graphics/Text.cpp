#include "Text.h"

Text::Text(Font &font, std::string text)
        : m_font(font), m_color(1.f), m_text(std::move(text))
{
    initSprites();
}

void Text::draw(SpriteBatch &batch)
{
    for (auto sprite : m_sprites)
    {
        // Шаманим, чтобы центр был в левом нижнем углу текста и работал ориджин
        sprite.setPosition(sprite.getPosition() + m_position + glm::vec2(0.f, m_height) - m_origin);
        sprite.setColor(m_color);
        batch.draw(sprite);
    }
}

std::string Text::getText() const
{
    return m_text;
}

// Нужно иметь в виду, что от текста зависят размеры
void Text::setText(const std::string& text)
{
    m_text = text;
    initSprites();
}

glm::vec2 Text::getPosition() const
{
    return m_position;
}

void Text::setPosition(glm::vec2 position)
{
    m_position = position;
}

glm::vec2 Text::getOrigin() const
{
    return m_origin;
}

void Text::setOrigin(glm::vec2 origin)
{
    m_origin = origin;
}

glm::vec4 Text::getColor() const
{
    return m_color;
}

void Text::setColor(glm::vec4 color)
{
    m_color = color;
}

float Text::getWidth() const
{
    return m_width;
}

float Text::getHeight() const
{
    return m_height;
}

void Text::initSprites()
{
    m_sprites.clear();
    Sprite sprite(m_font.getTexture());

    // Сразу при генерации узнаем размеры текста
    float maxWidth = 0;
    float maxHeight = 0;

    glm::vec2 pos(0.f);
    for (char c : m_text)
    {
        Character character = m_font.getCharacter(c);

        if (c == ' ')
        {
            // Размер пробела пусть зависит от размера шрифта
            pos += glm::vec2(m_font.getSize() / 4, 0.f);
            continue;
        }
        if (c == '\n')
        {
            // Тут шаманим, чтоб сделать отступ, вычислить ширину и высоту. Бррр..
            maxWidth = std::max(maxWidth, pos.x);
            maxHeight += (float) m_font.getSize();
            pos = glm::vec2(0.f, pos.y - (float) m_font.getSize());
            continue;
        }

        // Приходится отражать текстуру, потому что у freetype точка отчета находится слева вверху, а у нас слева внизу.
        // Если кто-то знает способ лучше, сообщите
        sprite.setTextureRect(
                IntRect(character.xOffset, character.size.y, character.size.x, -character.size.y)
        );
        // Не спрашивайте, почему тут такой ориджин.
        // По какой-то причине так оказалось удобнее
        sprite.setOrigin(glm::vec2(0.f, character.size.y));
        sprite.setWidth(character.size.x);
        sprite.setHeight(character.size.y);
        sprite.setPosition(pos);

        m_sprites.push_back(sprite);

        pos += glm::vec2(sprite.getWidth(), 0.f);
    }
    // Еще раз проверяем ширину для последней строки
    maxWidth = std::max(maxWidth, pos.x);
    // И добавляем высоту последней строки
    maxHeight += (float) m_font.getSize();

    m_width = maxWidth;
    m_height = maxHeight;
}

