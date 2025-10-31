#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/cglm.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FIXED_TYPES
#include <nuklear/nuklear.h>

#define NK_GLFW_GL3_IMPLEMENTATION
#include <nuklear/nuklear_glfw_gl3.h>

#include <windows.h>

#include "headers/Camera.h"
#include "headers/ModelLoader.h"

#define ARRAY_LEN(array) (sizeof(array) / (sizeof((array)[0])))


#define MODELS_INITIALAMOUNT 20

int selected_model = -1;
vec3 selected_color = {1.0f, 0.0f, 0.0f}; // Red for selected

char *GLOBAL_VERTEX_SHADER_PATH = "shaders/basicVertexShader.glsl";
char *GLOBAL_FRAGMENT_SHADER_PATH = "shaders/basicFragShader.frag";

Camera cam;
int WIDTH = 800; int HEIGHT = 800;
int n_models, n_lights;
Model_blueprint *models;
Model_blueprint *light_model;

int get_model_clicked(GLFWwindow* window, mat4 camMat) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    // Convert to normalized screen coordinates
    float mouseX = (xpos / WIDTH) * 2.0f - 1.0f;
    float mouseY = 1.0f - (ypos / HEIGHT) * 2.0f;
    
    for (int i = 0; i < n_models; i++) {
        // Transform model position to screen space
        vec4 model_pos = {models[i].position[0], models[i].position[1], models[i].position[2], 1.0f}; // Center of model
        vec4 screen_pos;
        glm_mat4_mulv(camMat, model_pos, screen_pos);
        
        // Perspective divide
        screen_pos[0] /= screen_pos[3];
        screen_pos[1] /= screen_pos[3];
        
        // Check if mouse is near model in screen space
        float dist = sqrtf(
            (screen_pos[0] - mouseX) * (screen_pos[0] - mouseX) +
            (screen_pos[1] - mouseY) * (screen_pos[1] - mouseY)
        );
        
        // If mouse is close to model in screen space, select it
        if (dist < 0.5f) { // Adjust this threshold as needed
            return i;
        }
    }
    
    return -1;
}

void handle_mouse_picking(GLFWwindow* window, mat4 camMat) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        selected_model = get_model_clicked(window, camMat);
        if (selected_model != -1) {
            // show controls
        } else {
            // hide controls
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    WIDTH = width;
    HEIGHT = height;
    
    // Update camera projection with new aspect ratio
    float aspect_ratio = (float)width / (float)height;
    camera_update_projection(&cam, 45, aspect_ratio, 0.3, 100.0);

    struct nk_glfw* glfw = (struct nk_glfw*)glfwGetWindowUserPointer(window);
    if (glfw) {
        glfw->width = width;
        glfw->height = height;
    }
}

/*
LRESULT CALLBACK GUI_CALLBACK(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar)
{
    switch (msg)
    {
    case WM_HSCROLL:
        if (selected_model != -1)
        {
            float rotX = SendMessage(hRotX, TBM_GETPOS, 0, 0);
            float rotY = SendMessage(hRotY, TBM_GETPOS, 0, 0);
            float rotZ = SendMessage(hRotZ, TBM_GETPOS, 0, 0);

            float posX = SendMessage(hPosX, TBM_GETPOS, 0, 0);
            posX /= 100;
            float posY = SendMessage(hPosY, TBM_GETPOS, 0, 0);
            posY /= 100;
            float posZ = SendMessage(hPosZ, TBM_GETPOS, 0, 0);
            posZ /= 100;

            // Apply rotations in your preferred order
            glm_mat4_identity(models[selected_model].model);

            glm_rotate(models[selected_model].model, glm_rad(rotX), (vec3){1, 0, 0});
            glm_rotate(models[selected_model].model, glm_rad(rotY), (vec3){0, 1, 0});
            glm_rotate(models[selected_model].model, glm_rad(rotZ), (vec3){0, 0, 1});

            RendererCopyVec3ToModel(&models[selected_model], (vec3){posX, posY, posZ});

            SetFocus(hWnd);
        }
        break;
    case WM_COMMAND:
        OPENFILENAMEA ofn;
        char szFile[260] = "";
        switch (LOWORD(wPar))
        {
        case MENU_LOADOBJMODEL:
            // Initialize structure completely
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "OBJ Files\0*.obj\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn))
            {
                int nf;
                OBJ_face *faces = LoadOBJ(ofn.lpstrFile, 1024, &nf);
                Model_blueprint model = RendererCreateObjModel(faces, nf, GLOBAL_VERTEX_SHADER_PATH, GLOBAL_FRAGMENT_SHADER_PATH);
                free(faces);

                if (n_models > MODELS_INITIALAMOUNT)
                {
                    printf("\n\nError: models amount exceeded 10\n\n\n");
                }
                else
                {
                    UniformSend4x4Matrix(model.shader_program, "model", model.model);
                    // TextureCreateTexture(&model.texture, model.shader_program, "tex0", 0, "assets/Tbrick.png");
                    TextureBindNoTexture(&model);
                    models[n_models] = model;
                    n_models++;
                }
            }
            break;
        case MENU_LOADANYMODEL:
            // Initialize structure completely
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn))
            {
                Assimp_object ass = LoadAssimp(ofn.lpstrFile);
                Model_blueprint *model_meshes = RendererCreateModel(ass, GLOBAL_VERTEX_SHADER_PATH, GLOBAL_FRAGMENT_SHADER_PATH);

                // UniformSend4x4Matrix(model.shader_program, "model", model.model);
                printf("Adding meshes to main array\n");
                for (size_t i = 0; i < ass.n_meshes; i++)
                {
                    TextureBindNoTexture(&model_meshes[i]);
                    models[n_models] = model_meshes[i];
                    n_models++;
                    printf("Added mesh %d\n", i);
                }

                printf("model should be loaded\n");
            }
            break;
        }
    }

    return CallWindowProc(gl_wnd_proc, hWnd, msg, wPar, lPar);
}
*/

int main(int argc, char *argv[])
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Lightweight 3D Program", NULL, NULL);
    if (!window) { printf("error in window\n"); return 1; }
    glfwMakeContextCurrent(window);

    struct nk_glfw glfw = {0};
    struct nk_context *ctx;
    struct nk_font_atlas *atlas;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetWindowUserPointer(window, &glfw);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("glad failed to load GL\n");
        return 1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    ctx = nk_glfw3_init(&glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&glfw, &atlas);
    nk_glfw3_font_stash_end(&glfw);

    
    GLfloat lightVertices[] =
    { //     COORDINATES     //
        -0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f, -0.1f,
        0.1f, -0.1f, -0.1f,
        0.1f, -0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f,
        0.1f,  0.1f, -0.1f,
        0.1f,  0.1f,  0.1f
    };

    GLuint lightIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
        0, 4, 7,
        0, 7, 3,
        3, 7, 6,
        3, 6, 2,
        2, 6, 5,
        2, 5, 1,
        1, 5, 4,
        1, 4, 0,
        4, 5, 6,
        4, 6, 7
    };

    // creating a simple light model
    models = malloc( sizeof(Model_blueprint) * MODELS_INITIALAMOUNT );

    VAOAttribute *t_attrs = VAOCreateVAOAttributeArrays(1);
    t_attrs[0] = (VAOAttribute){0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0};
    Model_blueprint c = RendererCreateModelAOS(
        lightVertices, sizeof(lightVertices), lightIndices, sizeof(lightIndices),
        t_attrs, 1, "shaders/lightVertexShader.glsl", "shaders/lightFragShader.frag"
    );

    free(t_attrs);

    RendererCopyVec3ToModel(&c, (vec3){-1.2f, 1.0f, 1.0f});
    UniformSend4x4Matrix(c.shader_program, "model", c.model);
    
    // create camera
    mat4 camMat;
    camera_init(&cam, (vec3){0.0f, 0.0f, 2.0f});

    // lighting
    vec4 lightC = {1.0f, 1.0f, 1.0f, 1.0f};

    // ofn used for loading model files
    OPENFILENAME ofn;
    char szFile[260] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    Assimp_object ass = LoadAssimp("D:/programming/LW3DP/LW3DP/models/simple_girl/girl OBJ.obj");
    Model_blueprint *models_created = RendererCreateModel(ass, GLOBAL_VERTEX_SHADER_PATH, GLOBAL_FRAGMENT_SHADER_PATH);

    for (size_t i = 0; i < ass.n_meshes; i++)
    {
        models[n_models] = models_created[i]; n_models++;
    }
    

    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        // /*
        nk_glfw3_new_frame(&glfw);
        if (
            nk_begin(ctx, "LW3DP", nk_rect(0, 0, (float)w, (float)h),
            NK_WINDOW_BORDER)
        )
        {
            // drawing the top menubar
            nk_menubar_begin(ctx);
            nk_layout_row_static(ctx, 25, 40, 5);

            if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 200)))
            {
                nk_layout_row_dynamic(ctx, 22, 1);

                if (nk_menu_item_label(ctx, "Load model", NK_TEXT_LEFT))
                {
                    if (GetOpenFileNameA(&ofn))
                    {
                        Assimp_object ass = LoadAssimp(ofn.lpstrFile);
                        Model_blueprint *models_created = RendererCreateModel(ass, GLOBAL_VERTEX_SHADER_PATH, GLOBAL_FRAGMENT_SHADER_PATH);

                        for (size_t i = 0; i < ass.n_meshes; i++)
                        {
                            models[n_models] = models_created[i]; n_models++;
                        }
                        
                    }
                }
                if (nk_menu_item_label(ctx, "Load OBJ", NK_TEXT_LEFT))
                {
                    ofn.lpstrFilter = "OBJ Files\0*.obj\0";
                    if (GetOpenFileNameA(&ofn))
                    {
                        Assimp_object ass = LoadAssimp(ofn.lpstrFile);
                        Model_blueprint *models_created = RendererCreateModel(ass, GLOBAL_VERTEX_SHADER_PATH, GLOBAL_FRAGMENT_SHADER_PATH);

                        for (size_t i = 0; i < ass.n_meshes; i++)
                        {
                            models[n_models] = models_created[i]; n_models++;
                        }
                        
                    }
                }

                nk_menu_end(ctx);
            }
            nk_menubar_end(ctx);

            nk_end(ctx);
            if (
                nk_begin(
                    ctx, "Tools", nk_rect(w-(w/4), 0, w/4, h),
                    NK_WINDOW_TITLE | NK_WINDOW_BORDER
                )
            )
            {

                nk_end(ctx);
            }

            nk_window_set_focus(ctx, "Tools");
        }
        // */
        
        glClearColor(0.35, 0.36, 0.23, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
        glEnable(GL_DEPTH_TEST);
        
        // Process input
        camera_process_keyboard(&cam, window);
        camera_process_mouse(&cam, window, WIDTH, HEIGHT);
        handle_mouse_picking(window, camMat);
        
        // Update camera matrices
        camera_update_view(&cam);
        float aspect = (float)WIDTH/(float)HEIGHT;
        camera_update_projection(&cam, 45, aspect, 0.3, 100.0);
        glm_mat4_mul(cam.projection, cam.view, camMat);

        // --- DRAW MODELS ---
        for (size_t i = 0; i < n_models; i++)
        {
            UniformSend4x4Matrix(models[i].shader_program, "camMat", camMat);
            UniformSend4x4Matrix(models[i].shader_program, "model", models[i].model);

            if (i == selected_model) {
                UniformSendVec4(models[i].shader_program, "lightC", (vec4){1.0f, 0.0f, 0.0f, 1.0f});
            } else {
                UniformSendVec4(models[i].shader_program, "lightC", lightC);
            }

            UniformSendVec3(models[i].shader_program, "lightPos", c.position);
            if (models[i].texture != 0)
            {
                TextureBindTexture(models[i], GL_TEXTURE_2D);
            }
            
            EBODraw(models[i].indices_count, GL_UNSIGNED_INT, models[i].VAO);
            //VAODraw(models[i].VAO, models[i].indices_count);
        }

        UniformSend4x4Matrix(c.shader_program, "camMat", camMat);
        UniformSend4x4Matrix(c.shader_program, "model", c.model);
        UniformSendVec4(c.shader_program, "lightC", lightC);
        EBODraw(ARRAY_LEN(lightIndices), GL_UNSIGNED_INT, c.VAO);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // CLEANUP
    /*
    glDeleteVertexArrays(1, &m.VAO);
    glDeleteBuffers(1, &m.VBO);
    glDeleteBuffers(1, &m.EBO);
    glDeleteTextures(1, &m.texture);
    glDeleteProgram(m.shader_program);
    */
    nk_glfw3_shutdown(&glfw);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}