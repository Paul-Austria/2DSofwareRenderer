#include <iostream>
#include <SoftRendererLib/src/include/SoftRenderer.h>

int main()
{
    Renderer2D::RenderContext2D context;

    // Initialize your context or textures as needed for testing
    // Example: context.SetTargetTexture(...);

    // Perform tests on SoftRendererLib functions
    // Example:
    context.ClearTarget(Renderer2D::Color(150, 150, 150));
    context.DrawRect(Renderer2D::Color(255, 0, 0), 10, 10, 200, 100);

    // Add performance measurement code here as needed

    std::cout << "SoftRendererLib tests completed." << std::endl;

    return 0;
}
