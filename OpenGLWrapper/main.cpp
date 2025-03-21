#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SoftRendererLib/src/include/SoftRenderer.h>
#include <SoftRendererLib/src/data/PixelFormat/PixelFormatInfo.h>
#include <fstream>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

// Constants
#define WIDTH 800
#define HEIGHT 400
const double TARGET_FPS = 6088.0;
const double TARGET_FRAME_DURATION = 1.0 / TARGET_FPS;

using namespace Tergos2D;

// Vertex Shader source code
const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

// Fragment Shader source code
const char *fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texture1;
    void main() {
        FragColor = texture(texture1, TexCoord);
    }
)";

// Vertex data for a full-screen quad
// Original texture coordinates for a full-screen quad
float vertices[] = {
    // positions      // texture coords (flipped)
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // Bottom-left corner
    1.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // Bottom-right corner
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // Top-right corner
    -1.0f, 1.0f, 0.0f, 0.0f, 0.0f   // Top-left corner
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0};

// Function prototypes
void generateRandomTextureData(unsigned char *data, int width, int height);
void generateAnimatedGradientTextureData(unsigned char *data, int width, int height, float time);
void TestingFunction();
void SetupFunc();
RenderContext2D context;
Texture *TargetTexture = nullptr;
int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Window", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Disable V-Sync

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Create and compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed: " << infoLog << std::endl;
        return -1;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed: " << infoLog << std::endl;
        return -1;
    }

    // Link shaders into a program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed: " << infoLog << std::endl;
        return -1;
    }

    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Setup vertex data, buffers, and configure vertex attributes
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Generate and configure texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate memory for texture data and generate initial texture
    unsigned char *imageData = new unsigned char[3 * WIDTH * HEIGHT];
    TargetTexture = new Texture(WIDTH, HEIGHT, imageData, PixelFormat::RGB24, 3 * WIDTH);
    context.SetTargetTexture(TargetTexture);

    //  generateRandomTextureData(imageData, WIDTH, HEIGHT);
    SetupFunc();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Main loop
    // Variables for FPS counting
    double previousTime = 0.0;
    int frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        // Start frame timer
        double frameStartTime = glfwGetTime();

        // Generate and update texture data
        TestingFunction();
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, imageData);

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // FPS calculation
        frameCount++;
        double currentTime = glfwGetTime();

        // Check if a second has passed
        if (currentTime - previousTime >= 1.0)
        {
            double fps = frameCount / (currentTime - previousTime);
            double timePerFrame = 1000.0 / fps; // Convert to milliseconds
            std::cout << "FPS: " << fps << " | Time per frame: " << timePerFrame << " ms" << std::endl;

            // Reset counters
            previousTime = currentTime;
            frameCount = 0;
        }

        // Calculate the frame duration and sleep if necessary
        double frameEndTime = glfwGetTime();
        double frameDuration = frameEndTime - frameStartTime;

        if (frameDuration < TARGET_FRAME_DURATION)
        {
            // Sleep for the remaining time
            double sleepTime = TARGET_FRAME_DURATION - frameDuration;
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
        }
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);
    delete[] imageData;
    delete TargetTexture;

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
Texture text;
Texture text2;
Texture text3;
Texture text5;
int imgwidth, imgheight, nrChannels;
uint8_t *data = nullptr;
uint8_t *data2 = nullptr;
uint8_t *data3 = nullptr;
Texture text4;
uint8_t *data4 = nullptr;
uint8_t *logo8 = nullptr;
Texture logo8Texture;
uint8_t *logo4 = nullptr;
Texture logo4Texture;
int imgwidth4 = 234;
int imgheight4 = 243;

void loadTexture(std::string texturePath, PixelFormat format, Texture &target, int height, int width)
{
    std::ifstream file(texturePath, std::ios::binary | std::ios::ate);
    if (file)
    {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        data4 = new uint8_t[size];
        if (file.read(reinterpret_cast<char *>(data4), size))
        {
            target = Texture(width, height, data4, format, 0);
        }
        else
        {
  //          std::cerr << "Failed to read" << texturePath << std::endl;
        }
        file.close();
    }
    else
    {
//        std::cerr << "Failed to open " << texturePath << std::endl;
    }
}
void SetupFunc()
{

    data = stbi_load("data/img1.png", &imgwidth, &imgheight, &nrChannels, 3);
    text = Texture(imgwidth, imgheight, data, PixelFormat::RGB24, 0);
    data2 = stbi_load("data/Candera.png", &imgwidth, &imgheight, &nrChannels, 4);
    text2 = Texture(imgwidth, imgheight, data2, PixelFormat::RGBA8888, 0);
    data3 = stbi_load("data/logo-de.png", &imgwidth, &imgheight, &nrChannels, 4);
    text3 = Texture(imgwidth, imgheight, data3, PixelFormat::RGBA8888, 0);
    data4 = stbi_load("data/images.png", &imgwidth, &imgheight, &nrChannels, 3);
    text5 = Texture(imgwidth, imgheight, data4, PixelFormat::RGB24, 0);

    // Load the binary file
    loadTexture("data/testrgb565.bin", PixelFormat::RGB565, text4, 234, 243);
    loadTexture("data/logo8.bin", PixelFormat::GRAYSCALE8, logo8Texture, 136, 500);

    context.SetBlendMode(BlendMode::SIMPLE);
}
static float x = 0;

void TestRotationWithOffset(RenderContext2D &context, Texture &texture, int xOffset = 0, int yOffset = 0);

// Function used for testing, updates the texture data
void TestingFunction()
{
    x += 0.5f;
    context.SetBlendFunc(BlendFunctions::BlendRow);
    context.ClearTarget(Color(150, 150, 150));
    context.SetClipping(80, 30, 370, 290);
    context.EnableClipping(false);
    context.primitivesRenderer.DrawLine(Color(255, 0, 0), 0, 0, 255, 33);
    context.primitivesRenderer.DrawLine(Color(0, 0, 255), 0, 33, 10, 33);
    context.SetClipping(80, 30, 370, 290);
    context.EnableClipping(false);
    context.SetSamplingMethod(SamplingMethod::NEAREST);
    context.primitivesRenderer.DrawRect(Color(255, 255, 255), 80, 30, 370, 290);
    // context.DrawTexture(text, 40, 40);
    context.basicTextureRenderer.DrawTexture(text2, -50, -30);
    //  context.DrawTexture(text5, 550, 190,0.2,2, SamplingMethod::NEAREST);
    context.transformedTextureRenderer.DrawTexture(text5, -1, 190, 2, 2, 0, 0, 0);
    //  context.transformedTextureRenderer.DrawTexture(text5, 350, 190,0.2,2);

    context.basicTextureRenderer.DrawTexture(text3, 50, 90);
    context.SetBlendFunc(BlendFunctions::BlendSolidRowRGB24);
    context.primitivesRenderer.DrawRect(Color(130, 90, 90, 90), 0, 0, WIDTH, HEIGHT);
    context.SetBlendFunc(BlendFunctions::BlendRow);

    context.primitivesRenderer.DrawRect(Color(0, 40, 150), 0, 0, 3000, 60);
    context.primitivesRenderer.DrawRotatedRect(Color(0, 150, 40), 0, 0, 400, 40, x);

    context.primitivesRenderer.DrawRect(Color(150, 200, 0, 0), 120, 0, 300, 90);

    context.primitivesRenderer.DrawRect(Color(150, 200, 100, 0), 0, 0, 100, 300);
    // context.primitivesRenderer.DrawRotatedRect(Color(255,200, 100, 255), 250, 250, 100, 300,x);

    // context.SetBlendMode(BlendMode::NOBLEND);
    Coloring st = {true, Color(155, 0, 255, 0)};
    context.SetColoringSettings(st);
    context.basicTextureRenderer.DrawTexture(logo8Texture, 90, 90 + 150);
    context.SetBlendFunc(BlendFunctions::BlendRGBA32ToRGB24);
    context.transformedTextureRenderer.DrawTexture(text5, 550, 150, 0.5, 0.5, x);
    context.SetBlendFunc(BlendFunctions::BlendRow);

    context.GetColoring().colorEnabled = false;
    context.SetBlendMode(BlendMode::SIMPLE);
    context.SetBlendMode(BlendMode::SIMPLE);
    /*
        for (size_t i = 0; i < 1; i++)
        {

            for (size_t y = 0; y < 30; y++)
            {

                for (int16_t x = 0; x < 40; x++)
                {
                    context.basicTextureRenderer.DrawTexture(text, x * 51, y * 52);
                }
            }
        }

        */
}

#include <iomanip>

void TestRotationWithOffset(RenderContext2D &context, Texture &texture, int xOffset, int yOffset)
{
    // Constants
    const int screenWidth = 800;
    const int screenHeight = 400;
    const int textureWidth = 60;
    const int textureHeight = 51;
    const int redSquareSize = 5;

    // Clear the screen
    context.ClearTarget(Color(150, 150, 150));

    // Rotation and Offset info grid
    std::cout << "Rotation Grid with Offsets (Degrees):\n";
    int rows = screenHeight / (20 + textureHeight);
    int cols = screenWidth / (20 + textureWidth);
    int rotation = -5;

    // Fill the screen with rotated textures
    for (int y = 0; y < screenHeight; y += textureHeight + 20)
    {
        for (int x = 0; x < screenWidth; x += textureWidth + 20)
        {
            // Compute rotation angle
            rotation += 5;

            rotation = rotation % 360;

            // Draw the texture with rotation
            //           context.transformedTextureRenderer.DrawTexture(texture, x, y, rotation, xOffset, yOffset);

            // Draw a red square at the same location as the center of the texture
            int squareX = x;
            int squareY = y;
            //  context.DrawRect(Color(255, 0, 0), squareX, squareY, redSquareSize, redSquareSize);

            // Print rotation angle and offsets in grid format
            std::cout << std::setw(6) << rotation;
        }
        std::cout << '\n'; // New line at the end of each row
    }
}
