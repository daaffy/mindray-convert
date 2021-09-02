#ifndef GUI_SDL2_WINDOW_HH
#define GUI_SDL2_WINDOW_HH

#include <memory>
#include <utility>
#include <functional>
#include <iostream>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

#include <SDL2/SDL.h>
#include <CL/cl2.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include "Instance.hh"
#include "Rectangle.hh"
#include "Kernel.hh"
#include "Renderer.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

namespace gui
{
    class Window
    {
    private:
        std::shared_ptr<events::EventManager> eventManager;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;

        SDL_GLContext glContext;

        // std::optional<SDL_Event> dragObject;

        void windowEvent(const SDL_Event &e)
        {
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_MINIMIZED:
                minimised = true;
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
                break;
            case SDL_WINDOWEVENT_RESTORED:
                minimised = false;
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                redraw(e);
                break;
            case SDL_WINDOWEVENT_MOVED:
                break;
            default:
                break;
            }
        }

        bool minimised = false;

        glm::mat4 projection;
        std::pair<float, float> size;

    public:
        std::vector<std::shared_ptr<gui::Rectangle>> drawables;
        std::vector<std::shared_ptr<gui::Renderer>> renderers;
        std::shared_ptr<gui::Kernel> kernel;

        Window(unsigned int width = 640, unsigned int height = 480, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE) : Window(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags) {}
        Window(unsigned int xPos, unsigned int yPos, unsigned int width, unsigned int height, Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE) : eventManager(std::make_shared<events::EventManager>()), window(nullptr, SDL_DestroyWindow), projection(1.0f), size(static_cast<float>(width), static_cast<float>(height))
        {
            window.reset(SDL_CreateWindow("Ultrasound Pre-Processor", xPos, yPos, width, height, flags));
            if (window == nullptr)
            {
                std::cout << "SDL2 window initialisation failed, error: " << SDL_GetError() << std::endl;
                return;
            }

            glContext = SDL_GL_CreateContext(window.get());
            if (glContext == nullptr)
            {
                std::cout << "OpenGL context initialisation failed, error: " << SDL_GetError() << std::endl;
                window.release();
                return;
            }

            eventManager->addCallback(SDL_WINDOWEVENT, std::bind(windowEvent, this, std::placeholders::_1));
        }

        ~Window()
        {
            SDL_GL_DeleteContext(glContext);
        }

        Window(const Window &) = delete;
        Window(Window &&) = default;

        void clean()
        {
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void update()
        {
        }

        void draw()
        {
            if (minimised)
            {
                return;
            }

            unsigned int rs = static_cast<unsigned int>(std::sqrt(renderers.size() - 1)) + 1;
            unsigned int rx = 0; // Move this stuff to time of addition at some point.
            unsigned int ry = 0;
            float rw = std::min(size.first, size.second) / static_cast<float>(rs);
            float xOff = size.first > size.second ? (size.first - size.second) / 2.0f : 0.0f;
            float yOff = size.second > size.first ? (size.second - size.first) / 2.0f : 0.0f;

            // This loop could be removed.
            for (auto &&renderer : renderers)
            {
                renderer->resize(static_cast<float>(rx) * rw + xOff - renderer->x, static_cast<float>(ry) * rw + yOff - renderer->y, rw - renderer->w, rw - renderer->h);
                renderer->draw();
                rx = (rx + 1) % rs;
                ry = rx ? ry : ry + 1;
            }

            for (auto &&drawable : drawables)
            {
                drawable->draw();
            }

            if (kernel)
                kernel->draw();

            SDL_GL_SwapWindow(window.get());
        }

        void redraw()
        {
            SDL_Event ev;
            ev.window.data1 = static_cast<Sint32>(size.first);
            ev.window.data2 = static_cast<Sint32>(size.second);
            redraw(ev);
        }

        // void drawArrow();
        void redraw(const SDL_Event &e)
        {
            if (minimised)
            {
                return;
            }

            // auto pair = getSize();

            glViewport(0, 0, e.window.data1, e.window.data2);

            std::pair<float, float> newSize = {e.window.data1, e.window.data2};

            SDL_Event ev;
            ev.user.type = events::GUI_REDRAW;
            ev.user.code = 0;
            ev.user.data1 = static_cast<void *>(&size);
            ev.user.data2 = static_cast<void *>(&newSize);

            for (auto &&drawable : drawables)
            {
                drawable->eventManager->process(ev);
            }

            size.first = static_cast<float>(e.window.data1);
            size.second = static_cast<float>(e.window.data2);
            // if (pair.first > pair.second)
            // {
            //     float aspect = static_cast<float>(pair.first) / static_cast<float>(pair.second);
            //     projection = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, 0.0f, 1.0f);
            // }
            // else
            // {
            //     float aspect = static_cast<float>(pair.second) / static_cast<float>(pair.first);
            //     projection = glm::ortho(-1.0f / aspect, 1.0f / aspect, -1.0f, 1.0f, 0.0f, 1.0f);
            // }

            projection = glm::ortho(0.0f, size.first, size.second, 0.0f, 0.0f, 1.0f);
            glUniformMatrix4fv(Instance::projectionUni, 1, GL_FALSE, reinterpret_cast<const GLfloat *>(&projection[0]));
        }
        // void setActive();
        auto getPosition() -> std::pair<int, int>
        {
            std::pair<int, int> p;
            SDL_GetWindowPosition(window.get(), &p.first, &p.second);
            return p;
        }

        auto getSize() -> std::pair<int, int>
        {
            std::pair<int, int> p;
            SDL_GetWindowSize(window.get(), &p.first, &p.second);
            return p;
        }

        auto getID() -> Uint32
        {
            return SDL_GetWindowID(window.get());
        }

        void eraseDrawable(std::size_t i)
        {
            drawables.erase(std::next(drawables.begin(), i));
        }

        void process(const SDL_Event &e)
        {
            eventManager->process(e);
            for (auto &&drawable : std::ranges::reverse_view(drawables)) // Reverse to interact with top drawn elements first.
            {

                if (drawable->hidden)
                    continue;

                if (events::containsMouse(std::as_const(*drawable), e))
                {
                    if (drawable->eventManager->process(e))
                    {
                        return;
                    }
                }
            }
            for (auto &&renderer : renderers) // Reverse to interact with top drawn elements first.
            {
                if (events::containsMouse(std::as_const(*renderer), e))
                {
                    if (renderer->eventManager->process(e))
                    {
                        break;
                    }
                }
            }
        }

        void addDrawable(std::shared_ptr<gui::Rectangle> &&d)
        {
            drawables.push_back(std::forward<std::shared_ptr<gui::Rectangle>>(d));
        }
    };

} // namespace gui

#endif