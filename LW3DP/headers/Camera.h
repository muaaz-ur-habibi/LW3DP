#ifndef TEST_CAMERA_H
#define TEST_CAMERA_H

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

// Struct to hold all camera data
typedef struct {
    vec3 position;
    vec3 orientation;
    vec3 up;

    mat4 view, projection;
    
    float pitch;
    float yaw;

    float movementSpeed;
    float mouseSensitivity;
} Camera;

// Function prototypes
void camera_init(Camera* cam, vec3 position);
void camera_update_vectors(Camera* cam);
void camera_update_view(Camera* cam);
void camera_process_keyboard(Camera* cam, GLFWwindow* window);
void camera_update_projection(Camera* cam, float fov_deg, float aspect, float near, float far);
void camera_process_mouse(Camera *cam, GLFWwindow *window, float width, float height);



#endif // TEST_CAMERA_H