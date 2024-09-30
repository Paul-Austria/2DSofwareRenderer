#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SoftRendererLib/src/include/SoftRenderer.h>

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
float vertices[] = {
    // positions      // texture coords
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

// Function prototypes
void generateRandomTextureData(unsigned char* data, int width, int height);
void generateAnimatedGradientTextureData(unsigned char* data, int width, int height, float time);
void TestingFunction(unsigned char* data);

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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Generate and update texture data
        TestingFunction(imageData);
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

// Function to fill a 2D array with random RGB data
void generateRandomTextureData(unsigned char* data, int width, int height) {
    const unsigned char startColor[3] = { 255, 0, 0 }; // Red
    const unsigned char endColor[3] = { 0, 0, 255 };   // Blue
    float step = 1.0f / (width - 1);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float t = x * step;
            unsigned char r = static_cast<unsigned char>(startColor[0] * (1 - t) + endColor[0] * t);
            unsigned char g = static_cast<unsigned char>(startColor[1] * (1 - t) + endColor[1] * t);
            unsigned char b = static_cast<unsigned char>(startColor[2] * (1 - t) + endColor[2] * t);
            int index = 3 * (y * width + x);
            data[index] = r;
            data[index + 1] = g;
            data[index + 2] = b;
        }
    }
}

// Function to fill a 2D array with animated gradient RGB data
void generateAnimatedGradientTextureData(unsigned char* data, int width, int height, float time) {
    unsigned char startColor[3] = {
        static_cast<unsigned char>(128 + 127 * std::sin(time)),
        static_cast<unsigned char>(128 + 127 * std::cos(time)),
        static_cast<unsigned char>(128 + 127 * std::sin(time + 3.14f))
    };
    unsigned char endColor[3] = {
        static_cast<unsigned char>(128 + 127 * std::cos(time + 1.57f)),
        static_cast<unsigned char>(128 + 127 * std::sin(time + 3.14f)),
        static_cast<unsigned char>(128 + 127 * std::cos(time))
    };

    float step = 1.0f / (width - 1);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float t = x * step;
            unsigned char r = static_cast<unsigned char>(startColor[0] * (1 - t) + endColor[0] * t);
            unsigned char g = static_cast<unsigned char>(startColor[1] * (1 - t) + endColor[1] * t);
            unsigned char b = static_cast<unsigned char>(startColor[2] * (1 - t) + endColor[2] * t);
            int index = 3 * (y * width + x);
            data[index] = r;
            data[index + 1] = g;
            data[index + 2] = b;
        }
    }
}


// Function used for testing, updates the texture data
void TestingFunction(unsigned char* data) {
    static float time = 0;
    time += 0.01f;

    context.ClearTarget(Color(0, 255,0));

    context.DrawRect(Color(0,0,255), 50, 50, 300, 30);
 //   generateAnimatedGradientTextureData(data, WIDTH, HEIGHT, time);


}
