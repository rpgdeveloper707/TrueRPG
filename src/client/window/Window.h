#ifndef RPG_WINDOW_H
#define RPG_WINDOW_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>

#include "../../utils/Event.hpp"

class Window;

class Window
{
private:
    using InputEvent = Event<const Window &, int, int>;
    using ResizeEvent = Event<int, int>;

    using IInputEvent = IEvent<const Window &, int, int>;
    using IResizeEvent = IEvent<int, int>;

    GLFWwindow *m_window;
    InputEvent m_onInput;
    ResizeEvent m_onResize;
    bool m_keys[GLFW_KEY_LAST + 1];
public:
    IResizeEvent &onResize;
    IInputEvent &onInput;

    bool isOpen() const;

    void close() const;

    void destroy() const;

    void swapBuffers() const noexcept;

    void pollEvents() const;

    int getWidth() const;

    int getHeight() const;

    bool getKey(int key);

    const ResizeEvent &getOnResize() const;

    static Window& getInstance(int width = 0, int height = 0, const std::string& title = "");

private:
    Window(int width, int height, const std::string &title);

    Window(const Window&) = delete;
    Window& operator= (const Window&) = delete;

    static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int actions, int mods);

    static void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height);
};

#endif //RPG_WINDOW_H
