#include "headers/Camera.h"
#include <cglm/cglm.h>
#include <math.h>

// Initialize the camera
void camera_init(Camera* cam, vec3 position) {
    glm_vec3_copy(position, cam->position);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->orientation); // Looks down negative Z
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam->up); // Up is positive Y

    cam->pitch = 0.0f;
    cam->yaw = -90.0f; // Initial yaw is often -90 degrees
    
    cam->movementSpeed = 0.05f;
    cam->mouseSensitivity = 30.0f;
}

// Update the camera's orientation vectors based on pitch and yaw
void camera_update_vectors(Camera* cam) {
    vec3 front;
    front[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    front[1] = sin(glm_rad(cam->pitch));
    front[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    
    glm_vec3_normalize_to(front, cam->orientation);
    
    // Recalculate the 'up' vector for the `lookAt` function
    glm_vec3_crossn(cam->orientation, (vec3){0.0f, 1.0f, 0.0f}, cam->up);
    glm_vec3_crossn(cam->up, cam->orientation, cam->up);
}

// Create the view matrix
void camera_update_view(Camera* cam) {
    vec3 pos_dir_sum;
    glm_vec3_add(cam->position, cam->orientation, pos_dir_sum);
    glm_lookat(cam->position, pos_dir_sum, cam->up, cam->view);
}

void camera_update_projection(Camera* cam, float fov_deg, float aspect, float near, float far) {
    glm_perspective(glm_rad(fov_deg), aspect, near, far, cam->projection);
}

// Process keyboard input
void camera_process_keyboard(Camera* cam, GLFWwindow* window) {
    float velocity = cam->movementSpeed;
    vec3 right;
    glm_vec3_crossn(cam->orientation, cam->up, right);

    // Forward and backward movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_muladds(cam->orientation, velocity, cam->position);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_muladds(cam->orientation, -velocity, cam->position);
    }

    // Strafe left and right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_muladds(right, -velocity, cam->position);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_muladds(right, velocity, cam->position);
    }

    // Vertical movement (up and down)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_muladds(cam->up, velocity, cam->position);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm_vec3_muladds(cam->up, -velocity, cam->position);
    }
}

void camera_process_mouse(Camera *cam, GLFWwindow *window, float width, float height)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		// Hides mouse cursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Stores the coordinates of the cursor
		double mouseX;
		double mouseY;
		// Fetches the coordinates of the cursor
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
		// and then "transforms" them into degrees 
		float rotX = cam->mouseSensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = cam->mouseSensitivity * (float)(mouseX - (width / 2)) / width;

		// Calculates upcoming vertical change in the Orientation
        vec3 normalized_right;  // the 'right' is the perpendicular component to
                                // orientation and up
        glm_cross(cam->orientation, cam->up, normalized_right);
        glm_normalize(normalized_right);
        glm_vec3_rotate(cam->orientation, glm_rad(-rotX), normalized_right);

		// Rotates the Orientation left and right
        glm_vec3_rotate(cam->orientation, glm_rad(-rotY), cam->up);

		// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		// Unhides cursor since camera is not looking around anymore
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}