#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SoftRendererLib/src/include/SoftRenderer.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"


// Constants
#define WIDTH 800
#define HEIGHT 400

// Vertex Shader source code
const char* vertexShaderSource = R"(
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
const char* fragmentShaderSource = R"(
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
    -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,  // Bottom-left corner
     1.0f, -1.0f, 0.0f,  1.0f, 1.0f,  // Bottom-right corner
     1.0f,  1.0f, 0.0f,  1.0f, 0.0f,  // Top-right corner
    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f   // Top-left corner
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

int imgwidth,imgheight,nrChannels;
uint8_t *data = nullptr;

// Function prototypes
void generateRandomTextureData(unsigned char* data, int width, int height);
void generateAnimatedGradientTextureData(unsigned char* data, int width, int height, float time);
void TestingFunction();
void SetupFunc();
RenderContext2D context;
Texture* TargetTexture = nullptr;
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
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
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed: " << infoLog << std::endl;
        return -1;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
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
    if (!success) {
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
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
    unsigned char* imageData = new unsigned char[3 * WIDTH * HEIGHT];
    TargetTexture = new Texture(WIDTH, HEIGHT, imageData, PixelFormat::RGB24);
    context.SetTargetTexture(TargetTexture);

  //  generateRandomTextureData(imageData, WIDTH, HEIGHT);
    SetupFunc();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
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




void CreateTestImage(uint8_t*& data, uint16_t width, uint16_t height) {
    const int bytesPerPixel = 3;  // RGB format, 3 bytes per pixel
    const int imageSize = width * height * bytesPerPixel; // Total size of the image data

    // Allocate memory for the image
    data = new uint8_t[imageSize];

    // Loop through each pixel and assign RGB values
    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            int pixelIndex = (y * width + x) * bytesPerPixel;

            // Create a simple pattern
            if ((x + y) % 2 == 0) {
                // Set this pixel to red (255, 0, 0)
                data[pixelIndex] = 255;     // Red
                data[pixelIndex + 1] = 0;   // Green
                data[pixelIndex + 2] = 0;   // Blue
            } else {
                // Set this pixel to blue (0, 0, 255)
                data[pixelIndex] = 0;       // Red
                data[pixelIndex + 1] = 0;   // Green
                data[pixelIndex + 2] = 255; // Blue
            }
        }
    }
}

void SetupFunc(){
    
    data = stbi_load("data/img1.jpg", &imgwidth, &imgheight, &nrChannels, 3); 
}



// Function used for testing, updates the texture data
void TestingFunction() {
    static float time = 0;
    time += 0.01f;

    context.ClearTarget(Color(150, 150,150));

    context.DrawRect(Color(0,40,150), 0, 0, 300, 40);
    context.DrawRect(Color(0,160,150), 60, 100, 300, 40);

    if(data != nullptr)
    {
        uint8_t vert[] = {
        250,40,0, 40,40,150,  40,40,150,  40,40,150,  40,40,150,
        250,40,0, 40,40,150,  40,40,150,  40,40,150,  40,40,150,
        250,40,0, 40,40,150,  40,40,150,  40,40,150,  40,40,150,
        250,40,0, 40,40,150,  40,40,150,  40,40,150,  40,40,150,
        250,40,0, 40,40,150,  40,40,150,  40,40,150,  40,40,150};
     //   context.DrawArray(vert, 40, 40, 5, 5, PixelFormat::RGB24);
        context.DrawArray(data, 40, 40, imgwidth, imgheight, PixelFormat::RGB24);
    }
 //   generateAnimatedGradientTextureData(data, WIDTH, HEIGHT, time);


}
