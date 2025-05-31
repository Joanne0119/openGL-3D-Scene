#pragma once
#include <GLFW/glfw3.h>

// 滑鼠按鈕按下後 callback function(回呼函式)
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// 滑鼠在指定視窗上移動時的 callback function(回呼函式)
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// 當視窗大小改變的 callback function
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

// 當鍵盤按鍵按下時的 callback function (回呼函式)
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
