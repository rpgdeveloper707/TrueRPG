#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include "window/Window.h"
#include "Player.h"
#include "graphics/Camera2D.h"
#include "Data.h"
#include "RTime.h"
#include "graphics/Texture.h"
#include "graphics/SpriteBatch.h"
#include "Collision.h"
#include "graphics/Font.h"
#include "graphics/Text.h"
#include "Game.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 800

// Никто не забыт, ничто не забыто

// D.E.L.E.T.E.D
// summonDestructor3000 
// 2021-2021
// Покойся с миром 

std::vector<std::unique_ptr<Tile>> TilesData;

void resizeCallback(Window *window, int width, int height);


int main()
{
    // Создание окна
    auto &window = Window::getInstance(SCR_WIDTH, SCR_HEIGHT, "TRUE RPG");
    window.makeContextCurrent();
    window.setResizeCallback(resizeCallback);

    Game game;

    // TODO: Пока сделал такой костыль, чтобы рисовать карту
    Camera2D* camera = game.getCamera();

    Texture emptyTexture;
    Texture wallTexture = Texture::create("../res/textures/enemy.png");

    // Подготовка спрайтов
    Sprite emptySprite(emptyTexture);
    emptySprite.setTextureRect(IntRect(0, 0, 0, 0));
    emptySprite.setScale({0, 0});
    emptySprite.setOrigin(glm::vec2(0));

    Sprite wallSprite(wallTexture);
    wallSprite.setTextureRect(IntRect(0, 224, 96, 96));
    wallSprite.setScale(glm::vec2(2.f / 3.f, 2.f / 3.f));
    wallSprite.setOrigin(glm::vec2(48, 48));

    TilesData.emplace_back(std::make_unique<Tile>(TileType::EMPTY, emptySprite, 0, false));
    TilesData.emplace_back(std::make_unique<Tile>(TileType::WALL, wallSprite, 1, true));

    const int animIndexArray = 3;
    int animCount = 0;
    float deltaTime = 0;
    float animationDelta = 0;

    // Game timer.
    RTime chrono(0.0f, 0.0f, 0.0f);
    Collision collision;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (window.isOpen())
    {
        camera->update();
        // Рендеринг
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        deltaTime = chrono.getDeltaTime();

        game.update(deltaTime);

        // glfw: обмен содержимым front- и back- буферов.
        // Отслеживание событий ввода/вывода (была ли нажата/отпущена кнопка, перемещен курсор мыши и т.п.)
        window.swapBuffers();
        glfwPollEvents();
    }

    wallTexture.destroy();
    game.destroy();

    window.destroy();

    return 0;
}

// всякий раз, когда изменяются размеры окна (пользователем или операционной системой), вызывается данная callback-функция
void resizeCallback(Window *window, int width, int height)
{
    // Убеждаемся, что окно просмотра соответствует новым размерам окна.
    // Обратите внимание, что высота и ширина будут значительно больше, чем указано, на Retina-дисплеях
    glViewport(0, 0, width, height);
}