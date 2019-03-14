#include "app.h"
#include "backdrop.h"
#include "camera.h"
#include "loader.h"
#include "log.h"
#include "mat.h"
#include "model.h"

void key_callback(GLFWwindow* window, int key, int scancode,
                  int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    app_t* app = glfwGetWindowUserPointer(window);
    camera_update_proj(app->camera, width, height);
    app_run(app);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    app_t* app = glfwGetWindowUserPointer(window);

    float out[4][4];
    mat4_mul(app->model->mat, app->camera->proj, out);
    float inv[4][4];
    mat4_inv(out, inv);

    float pos_screen[3] = { 2.0 * xpos / (app->camera->width) - 1.0f,
                            2.0 * ypos / (app->camera->height) - 1.0f,
                            0.0f};
    float pos_world[3];
    mat4_apply(inv, pos_screen, pos_world);

    log_trace("screen pos: %f %f\tworld pos: %f %f", pos_screen[0], pos_screen[1],
            pos_world[0], pos_world[1]);
}

void app_init(app_t* app) {
    glfwSetWindowUserPointer(app->window, app);
    glfwSetKeyCallback(app->window, key_callback);
    glfwSetWindowSizeCallback(app->window, window_size_callback);
    glfwSetCursorPosCallback(app->window, cursor_pos_callback);

    /*  We populated camera width + height, but didn't yet build
     *  the project matrix (to save time).  */
    camera_update_proj(app->camera, app->camera->width, app->camera->height);

    glClearDepth(1.0);
}

void app_run(app_t* app) {
    glClear(GL_DEPTH_BUFFER_BIT);
    backdrop_draw(app->backdrop);

    /*  At the very last moment, check on the loader */
    if (app->state == APP_PRELOAD) {
        loader_wait(app->loader, LOADER_DONE);
        loader_finish(app->loader, app->model);
    } else if (app->state == APP_LOAD &&
               loader_state(app->loader) == LOADER_DONE) {
        loader_finish(app->loader, app->model);
        loader_reset(app->loader);
        app->state = APP_RUNNING;
    }

    model_draw(app->model, app->camera->proj);
    glfwSwapBuffers(app->window);

    /*  Print a timing message on first load */
    if (app->state == APP_PRELOAD) {
        log_info("First draw complete");
        loader_reset(app->loader);
        app->state = APP_PREDRAW;
    }

    /*  On first draw (whether or not there was a model passed in on
     *  the command line), update the camera projection matrix using the
     *  true window size. */
    if (app->state == APP_PREDRAW) {
        int width;
        int height;
        glfwGetWindowSize(app->window, &width, &height);
        camera_update_proj(app->camera, width, height);
        log_trace("Got framebuffer size %i x %i", width, height);

        app->state = APP_RUNNING;
    }
}
