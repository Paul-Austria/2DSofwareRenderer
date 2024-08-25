#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <iostream>  // For std::cout
#include "../data/Texture.h"
#include "../data/Color.h"

class RenderContext2D
{

public:
    RenderContext2D() = default;
    ~RenderContext2D() = default;

    void SetTargetTexture(Texture* targettexture);
    void ClearTarget(Color color);

    static void Test() { std::cout << "hello world"; }

private:
    Texture* targetTexture = nullptr;
};


#endif