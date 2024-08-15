#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <SoftRendererLib/src/include/SoftRenderer.h>

#define WIDTH 1600
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

// 2D array data (e.g., a simple gradient)

#include <cstdlib> // for rand() and RAND_MAX
#include <ctime>   // for time()

// Function to fill a 2D array with gradient RGB data
void generateRandomTextureData(unsigned char* data, int width, int height) {
    // Define the gradient colors (start and end colors)
    const unsigned char startColor[3] = { 255, 0, 0 }; // Red
    const unsigned char endColor[3] = { 0, 0, 255 };   // Blue

    // Compute the gradient step
    float step = 1.0f / (width - 1);

    // Fill the array with gradient RGB values
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float t = x * step; // interpolation factor between 0 and 1

            // Linear interpolation between startColor and endColor
            unsigned char r = static_cast<unsigned char>(startColor[0] * (1 - t) + endColor[0] * t);
            unsigned char g = static_cast<unsigned char>(startColor[1] * (1 - t) + endColor[1] * t);
            unsigned char b = static_cast<unsigned char>(startColor[2] * (1 - t) + endColor[2] * t);

            // Set the pixel color
            int index = 3 * (y * width + x);
            data[index] = r;     // R
            data[index + 1] = g; // G
            data[index + 2] = b; // B
        }
    }
}

void generateAnimatedGradientTextureData(unsigned char* data, int width, int height, float time) {
    // Define the gradient colors (start and end colors) that change with time
    unsigned char startColor[3] = {
        static_cast<unsigned char>(128 + 127 * std::sin(time)), // R component
        static_cast<unsigned char>(128 + 127 * std::cos(time)), // G component
        static_cast<unsigned char>(128 + 127 * std::sin(time + 3.14f)) // B component
    };
    unsigned char endColor[3] = {
        static_cast<unsigned char>(128 + 127 * std::cos(time + 1.57f)), // R component
        static_cast<unsigned char>(128 + 127 * std::sin(time + 3.14f)), // G component
        static_cast<unsigned char>(128 + 127 * std::cos(time)) // B component
    };

    // Compute the gradient step
    float step = 1.0f / (width - 1);

    // Fill the array with gradient RGB values
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float t = x * step; // interpolation factor between 0 and 1

            // Linear interpolation between startColor and endColor
            unsigned char r = static_cast<unsigned char>(startColor[0] * (1 - t) + endColor[0] * t);
            unsigned char g = static_cast<unsigned char>(startColor[1] * (1 - t) + endColor[1] * t);
            unsigned char b = static_cast<unsigned char>(startColor[2] * (1 - t) + endColor[2] * t);

            // Set the pixel color
            int index = 3 * (y * width + x);
            data[index] = r;     // R
            data[index + 1] = g; // G
            data[index + 2] = b; // B
        }
    }
}


int main() {
    Context2D::Test();
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
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Create and compile the vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed: " << infoLog << std::endl;
        return -1;
    }

    // Create and compile the fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed: " << infoLog << std::endl;
        return -1;
    }

    // Link shaders into a shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed: " << infoLog << std::endl;
        return -1;
    }

    // Clean up shaders as they are no longer needed after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Generate and bind a Vertex Array Object and a Vertex Buffer Object
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

    // Generate and bind a texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate memory for texture data
    unsigned char* imageData = new unsigned char[3 * WIDTH * HEIGHT];

    // Fill the array with random RGB data
    generateRandomTextureData(imageData, WIDTH, HEIGHT);

    // Load image data into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);


    float time = 0;
    // Main loop
    while (!glfwWindowShouldClose(window)) {

        time += 0.01f;

        // Generate updated texture data
        generateAnimatedGradientTextureData(imageData, WIDTH, HEIGHT, time);

        // Update the texture with new data
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, imageData);



        // Render here
        glClear(GL_COLOR_BUFFER_BIT);

        // Use shader program and bind texture
        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Render the quad
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
