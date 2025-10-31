clear
gcc -o build/main.exe main.c EBO.c VAO.c VBO.c stb_image.c Textures.c Uniform.c Camera.c Renderer.c glad.c Shaders.c GUI.c ModelLoader.c -lglfw3 -lcomdlg32 -lcomctl32 -lassimp
./build/main.exe