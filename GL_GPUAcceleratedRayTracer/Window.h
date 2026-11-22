#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <string>
#include <GLFW/glfw3.h>
#include <glm.hpp>

using namespace std;
using namespace glm;

class Window {
public:
	Window(int w, int h, string winlabel);
	~Window();
	bool pressed(GLenum key) {
		return glfwGetKey(window, key);
	}
	bool getMouseMoved() {
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		mousemoving = abs(xChange) > 0 || abs(yChange) > 0;
		return mousemoving;
	}
	bool shouldClose();
	void swapNpoll();
	vec2 getMouseChange() { 
		vec2 mouseChangevec = vec2(xChange, yChange);
		xChange = 0;
		yChange = 0;
		return mouseChangevec;
	}
	static void keyCallback(GLFWwindow* awindow, int key, int scancode, int action, int mode);
	static void mousePosCallback(GLFWwindow* window, double xPos, double yPos);
	static void screenResizeCallback(GLFWwindow* awindow, int w, int h);
private:
	GLFWwindow* window;
	int width, height,
		bufferwidth, bufferheight;
    float lastX, lastY, mouseFirstMove,
        mouseX, mouseY, xChange, yChange;
	bool keypressed;
	bool mousemoving;
};