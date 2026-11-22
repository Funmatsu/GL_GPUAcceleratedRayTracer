#include "Window.h"
#include <iostream>

using namespace std;

Window::Window(int w, int h, string windowLabel){
	width = w; height = h;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	window = glfwCreateWindow(width, height,
							  windowLabel.c_str(),
							  NULL,/*create and configure full screen windows*/
							  NULL);

	glfwGetFramebufferSize(window, &bufferwidth, &bufferheight);
	cout << bufferwidth << " w: " << bufferheight << " h: " << endl;

	//set callback functions
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetFramebufferSizeCallback(window, screenResizeCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);

	glewInit();  
}
Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
	printf("Window dint make it after all!\n");
}

bool Window::shouldClose() { return glfwWindowShouldClose(window); }
void Window::swapNpoll() { glfwSwapBuffers(window); glfwPollEvents(); }

void Window::keyCallback(GLFWwindow* awindow, int key, int scancode, int action, int mode) {
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(awindow));

	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(win->window, true);
	}
	if (action == GLFW_PRESS) {
		win->keypressed = true;
	}
	else {
		win->keypressed = false;
	}
}

void Window::screenResizeCallback(GLFWwindow* awindow, int w, int h) {
	glViewport(0, 0, w, h);
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(awindow));
	//std::cout << win->bufferheight << endl;
	cout << w << " w: " << h << " h: " << endl;
	win->swapNpoll();
}

void Window::mousePosCallback(GLFWwindow* window, double xPos, double yPos) {
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMove) {
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMove = false;
	}

	theWindow->xChange = (xPos)-(theWindow->lastX);
	theWindow->yChange = (yPos)-(theWindow->lastY);


	theWindow->lastX = xPos;
	theWindow->lastY = yPos;

	theWindow->mouseX = xPos;
	theWindow->mouseY = yPos;
}

