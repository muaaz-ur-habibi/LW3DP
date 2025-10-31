#include <glad/glad.h>
#define  GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/cglm.h>

#include "headers/Camera.h"
#include "headers/GUI.h"
#include "headers/ModelLoader.h"

#define ARRAY_LEN(array) (sizeof(array) / (sizeof((array)[0])))

#define MENU_LOADOBJMODEL 1
#define MENU_LOADANYMODEL 2
#define MENU_TRANSFORMROTATE 6
#define MENU_TRANSFORMPOSITION 7
#define MENU_TRANSFORMSCALE 8

#define MENU_ROTXSLIDER 3
#define MENU_ROTYSLIDER 4
#define MENU_ROTZSLIDER 5
#define MENU_POSXSLIDER 9
#define MENU_POSYSLIDER 10
#define MENU_POSZSLIDER 11

#define MODELS_INITIALAMOUNT 20

int selected_model = -1;
vec3 selected_color = {1.0f, 0.0f, 0.0f}; // Red for selected

char *GLOBAL_VERTEX_SHADER_PATH = "shaders/basicVertexShader.glsl";
char *GLOBAL_FRAGMENT_SHADER_PATH = "shaders/basicFragShader.frag";

Camera cam;
int WIDTH = 800; int HEIGHT = 800;
static WNDPROC gl_wnd_proc;
int n_models, n_lights;
Model_blueprint *models;
Model_blueprint *light_model;

HWND hRotX, hRotY, hRotZ, hPosX, hPosY, hPosZ, hScaleX, hScaleY, hScaleZ;


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
            ShowWindow(hRotX, SW_SHOW); ShowWindow(hRotY, SW_SHOW); ShowWindow(hRotZ, SW_SHOW);
            ShowWindow(hPosX, SW_SHOW); ShowWindow(hPosY, SW_SHOW); ShowWindow(hPosZ, SW_SHOW);
        } else {
            ShowWindow(hRotX, SW_HIDE); ShowWindow(hRotY, SW_HIDE); ShowWindow(hRotZ, SW_HIDE);
            ShowWindow(hPosX, SW_HIDE); ShowWindow(hPosY, SW_HIDE); ShowWindow(hPosZ, SW_HIDE);
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
}


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

            float posX = SendMessage(hPosX, TBM_GETPOS, 0, 0); posX/=100;
            float posY = SendMessage(hPosY, TBM_GETPOS, 0, 0); posY/=100;
            float posZ = SendMessage(hPosZ, TBM_GETPOS, 0, 0); posZ/=100;
            
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
                } else {
                    UniformSend4x4Matrix(model.shader_program, "model", model.model);
                    //TextureCreateTexture(&model.texture, model.shader_program, "tex0", 0, "assets/Tbrick.png");
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


int main()
{
    INITCOMMONCONTROLSEX ccex = ccInit();
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Lightweight 3D Program", NULL, NULL);
    if (!window) { printf("error in window\n"); return 1; }
    glfwMakeContextCurrent(window);

    HWND hWnd = glfwGetWin32Window(window);
    
    hRotX = CreateWindowA(
        TRACKBAR_CLASSA, "", WS_CHILD | TBS_AUTOTICKS | TBS_HORZ,
        20, 20, 200, 50, hWnd, (HMENU)MENU_ROTXSLIDER, GetModuleHandle(NULL), NULL
    ); SendMessageA(hRotX, TBM_SETRANGE, TRUE, MAKELONG(0, 360)); SendMessageA(hRotX, TBM_SETPAGESIZE, 0, 10);
    hRotY = CreateWindowA(
        TRACKBAR_CLASSA, "", WS_CHILD | TBS_AUTOTICKS | TBS_HORZ,
        20, 72, 200, 50, hWnd, (HMENU)MENU_ROTYSLIDER, GetModuleHandle(NULL), NULL
    ); SendMessageA(hRotY, TBM_SETRANGE, TRUE, MAKELONG(0, 360)); SendMessageA(hRotY, TBM_SETPAGESIZE, 0, 10);
    hRotZ = CreateWindowA(
        TRACKBAR_CLASSA, "", WS_CHILD | TBS_AUTOTICKS | TBS_HORZ,
        20, 124, 200, 50, hWnd, (HMENU)MENU_ROTZSLIDER, GetModuleHandle(NULL), NULL
    ); SendMessageA(hRotZ, TBM_SETRANGE, TRUE, MAKELONG(0, 360)); SendMessageA(hRotZ, TBM_SETPAGESIZE, 0, 10);

    hPosX = CreateWindowA(
        TRACKBAR_CLASSA, "", WS_CHILD | TBS_AUTOTICKS | TBS_HORZ,
        20, 200, 200, 50, hWnd, (HMENU)MENU_POSXSLIDER, GetModuleHandle(NULL), NULL
    ); SendMessageA(hPosX, TBM_SETRANGE, TRUE, MAKELONG(0, 300)); SendMessageA(hPosX, TBM_SETPAGESIZE, 0, 10);
    hPosY = CreateWindowA(
        TRACKBAR_CLASSA, "", WS_CHILD | TBS_AUTOTICKS | TBS_HORZ,
        20, 252, 200, 50, hWnd, (HMENU)MENU_POSYSLIDER, GetModuleHandle(NULL), NULL
    ); SendMessageA(hPosY, TBM_SETRANGE, TRUE, MAKELONG(0, 300)); SendMessageA(hPosY, TBM_SETPAGESIZE, 0, 10);
    hPosZ = CreateWindowA(
        TRACKBAR_CLASSA, "", WS_CHILD | TBS_AUTOTICKS | TBS_HORZ,
        20, 304, 200, 50, hWnd, (HMENU)MENU_POSZSLIDER, GetModuleHandle(NULL), NULL
    ); SendMessageA(hPosZ, TBM_SETRANGE, TRUE, MAKELONG(0, 300)); SendMessageA(hPosZ, TBM_SETPAGESIZE, 0, 10);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("glad failed to load GL\n");
        return 1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    char *loading_menu_items[] = {
        "Load .obj model", "Load any model"
    };
    char *transform_menu_items[] = {
        "Rotate model", "Position model", "Scale model"
    };
    UINT_PTR loading_item_ids[] = {
        MENU_LOADOBJMODEL, MENU_LOADANYMODEL
    };
    UINT_PTR transform_item_ids[] = {
        MENU_TRANSFORMROTATE, MENU_TRANSFORMPOSITION, MENU_TRANSFORMSCALE
    };
    
    HMENU loading_sub_menu = CreateSubMenu(loading_menu_items, loading_item_ids, 2);
    HMENU transform_sub_menu = CreateSubMenu(transform_menu_items, transform_item_ids, 3);

    HMENU sub_menus[] = {
        loading_sub_menu, transform_sub_menu
    };
    char *sub_menu_names[] = {
        "&Model", "&Transform"
    };

    HMENU main_menu = CreateMenuBar(hWnd, sub_menus, sub_menu_names, 2);

    RECT rt = {0, 0, WIDTH, HEIGHT};
    AdjustWindowRect(&rt, GetWindowLongA(hWnd, GWL_STYLE), TRUE);
    MoveWindow(hWnd, 0, 0, rt.right-rt.left, rt.bottom-rt.top, TRUE);

    gl_wnd_proc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)GUI_CALLBACK);

    int len_of_single_vertex;
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

    // creating the models
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

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.35, 0.36, 0.23, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
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
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}