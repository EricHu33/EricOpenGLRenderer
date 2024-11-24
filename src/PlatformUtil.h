#pragma once
#include <string>
#include <GLFW/glfw3.h>

namespace Util
{
    class FileDialogs
    {
    public:
        static GLFWwindow* window;
        static std::string OpenFile(const char* path);
        static std::string SaveFile(const char* path);
    };
}
