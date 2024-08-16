#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <iostream>  // For std::cout
#include "../data/Texture.h"
#include "../data/Color.h"
class Context2D
{

public:
    Context2D() = default;
    ~Context2D() = default;

    void SetTargetTexture(Texture* targettexture);
    void ClearTarget(Color color);

    static void Test() { std::cout << "hello world"; }

private:
    Texture* targetTexture;
};


#endif