Clear-Host
$gcc_output = gcc -o build/main.exe main.c EBO.c VAO.c VBO.c stb_image.c Textures.c Uniform.c Camera.c Renderer.c glad.c Shaders.c ModelLoader.c -lglfw3 -lopengl32 -lcomdlg32 -lcomctl32 -lassimp -lshlwapi
Write-Output $gcc_output
./build/main.exe