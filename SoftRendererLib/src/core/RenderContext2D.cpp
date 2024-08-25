#include "RenderContext2D.h"

void RenderContext2D::SetTargetTexture(Texture *targettexture)
{
    this->targetTexture = targettexture;
}

void RenderContext2D::ClearTarget(Color color)
{
    if(targetTexture != nullptr)
    {
        
    }
}
