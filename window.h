#include "platform.h"

struct instance_;

GLFWwindow* window_new(float width, float height);
void window_delete(GLFWwindow* window);
void window_set_callbacks(GLFWwindow* window, struct instance_* instance);
