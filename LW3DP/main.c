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

#include "headers/Camera.h"
#include "headers/ModelLoader.h"

#define ARRAY_LEN(array) (sizeof(array) / (sizeof((array)[0])))


#define MODELS_INITIALAMOUNT 2000
#define LIGHTS_INITIALAMOUNT 10

int selected_model = -1, selected_light = -1;
vec4 selected_color = {1.0f, 1.0f, 0.0f, 1.0f}; // selected color

char *GLOBAL_VERTEX_SHADER_PATH = "shaders/basicVertexShader.glsl";
char *GLOBAL_FRAGMENT_SHADER_PATH = "shaders/basicFragShader.frag";

char *GLOBAL_LIGHT_VERTEX_SHADER = "shaders/lightVertexShader.glsl";
char *GLOBAL_LIGHT_FRAGMENT_SHADER = "shaders/lightFragShader.frag";

int WIREFRAME_MODE = 0;

Camera cam;
int WIDTH = 1280; int HEIGHT = 720;
int n_models, n_lights;
Model_blueprint *models;
Model_blueprint *light_models;

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

    // initialise the models array
    models = malloc( sizeof(Model_blueprint) * MODELS_INITIALAMOUNT );

    // initialise the lights array
    light_models = malloc( sizeof(Model_blueprint) * LIGHTS_INITIALAMOUNT );

    VAOAttribute *t_attrs = VAOCreateVAOAttributeArrays(1);
    t_attrs[0] = (VAOAttribute){0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0};
    Model_blueprint c = RendererCreateModelAOS(
        lightVertices, sizeof(lightVertices), lightIndices, sizeof(lightIndices),
        t_attrs, 1, GLOBAL_LIGHT_VERTEX_SHADER, GLOBAL_LIGHT_FRAGMENT_SHADER
    );
    free(t_attrs);

    RendererCopyVec3ToModel(&c, (vec3){-1.2f, 1.0f, 1.0f});
    UniformSend4x4Matrix(c.shader_program, "model", c.model);
    
    // create camera
    mat4 camMat;
    camera_init(&cam, (vec3){0.0f, 0.0f, 5.0f});

    // lighting
    glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 1.0f}, c.color);

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

    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        glClearColor(0.176, 0.176, 0.188, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Process input
        camera_process_keyboard(&cam, window);
        camera_process_mouse(&cam, window, WIDTH, HEIGHT);
        
        // Update camera matrices
        camera_update_view(&cam);
        float aspect = (float)WIDTH/(float)HEIGHT;
        camera_update_projection(&cam, 45, aspect, 0.1, 100.0);
        glm_mat4_mul(cam.projection, cam.view, camMat);

        /*
            DRAWING THE MODELS
        */
        
        if (WIREFRAME_MODE)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        for (size_t i = 0; i < n_models; i++)
        {
            UniformSend4x4Matrix(models[i].shader_program, "camMat", camMat);
            UniformSend4x4Matrix(models[i].shader_program, "model", models[i].model);

            if (i == selected_model) {
                UniformSendVec4(models[i].shader_program, "lightC", selected_color);
            } else {
                UniformSendVec4(models[i].shader_program, "lightC", c.color);
            }

            UniformSendVec3(models[i].shader_program, "lightPos", c.position);

            if (models[i].texture != 0)
            {
                TextureBindTexture(models[i], GL_TEXTURE_2D);
            }
            
            EBODraw(models[i].indices_count, GL_UNSIGNED_INT, models[i].VAO);
        }

        UniformSend4x4Matrix(c.shader_program, "camMat", camMat);
        UniformSend4x4Matrix(c.shader_program, "model", c.model);
        UniformSendVec4(c.shader_program, "lightC", c.color);
        EBODraw(ARRAY_LEN(lightIndices), GL_UNSIGNED_INT, c.VAO);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        /*
            DRAWING THE GUI
        */
        struct nk_style *s;
        s = &ctx->style;

        nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
        nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
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

                nk_menu_end(ctx);
            }
            if (nk_menu_begin_label(ctx, "Models", NK_TEXT_LEFT, nk_vec2(120, 200)))
            {
                nk_layout_row_dynamic(ctx, 22, 1);

                if (nk_menu_item_label(ctx, "Create light", NK_TEXT_LEFT))
                {
                    
                }

                nk_menu_end(ctx);
            }
            nk_menubar_end(ctx);

            nk_end(ctx);
        }
        nk_style_pop_color(ctx);
        nk_style_pop_style_item(ctx);

        // /*
        s = &ctx->style;
        nk_style_push_color(ctx, &s->window.background, nk_rgba(100, 0, 0, 255));
        nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(100, 0, 0, 255)));

        if (
            nk_begin(
                ctx, "Toolkit", nk_rect(w-(w/4), 0, w/4, h),
                NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_BACKGROUND
            )
        )
        {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "", NK_TEXT_CENTERED);

            nk_layout_row_static(ctx, 200, nk_window_get_width(ctx), 1);

            struct nk_list_view meshes_list;
            if (
                nk_list_view_begin(ctx, &meshes_list, "Lmeshes", NK_WINDOW_BORDER, 20, n_models)
            )
            {
                for (size_t i = meshes_list.begin; i < meshes_list.end; i++)
                {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    if (nk_button_label(ctx, models[i].model_name))
                    {
                        selected_model = i;
                    }
                }

                nk_list_view_end(&meshes_list);

                nk_layout_row_dynamic(ctx, 20, 1);
                nk_checkbox_label(ctx, "Wireframe mode", &WIREFRAME_MODE);
            }

            struct nk_list_view lights_list;
            if (
                nk_list_view_begin(ctx, &meshes_list, "Llights", NK_WINDOW_BORDER, 20, n_lights)
            )
            {
                for (size_t i = lights_list.begin; i < lights_list.end; i++)
                {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    if (nk_button_label(ctx, light_models[i].model_name))
                    {
                        selected_light = i;
                    }
                }

                nk_list_view_end(&meshes_list);
            }

            nk_end(ctx);   
        }
        nk_style_pop_color(ctx);
        nk_style_pop_style_item(ctx);

        nk_window_set_focus(ctx, "Toolkit");
        // */
        
        nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
        glEnable(GL_DEPTH_TEST); // for some reason nuklear disables this and forgets to reenable
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // CLEANUP
    
    
    nk_glfw3_shutdown(&glfw);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}