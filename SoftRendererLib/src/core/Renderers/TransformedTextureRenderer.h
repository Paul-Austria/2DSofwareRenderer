#ifndef TRANSFORMEDTEXTURERENDERER_H
#define TRANSFORMEDTEXTURERENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"
#include "../../data/Texture.h"
#include <functional>

namespace Tergos2D
{
    class TransformedTextureRenderer : RendererBase
    {
    public:
        TransformedTextureRenderer(RenderContext2D &context);
        ~TransformedTextureRenderer() = default;


        void DrawTexture(Texture &texture, int16_t x, int16_t y, float angle, int16_t offsetX = 0, int16_t offsetY = 0);

        void DrawTextureAndScale(Texture &texture, int16_t x, int16_t y,
                         float scaleX, float scaleY, float angle,
                         int16_t offsetX = 0, int16_t offsetY = 0);

        void DrawTexture(Texture &texture,  const float transformationMatrix[3][3]);

    private:
    };

} // namespace Tergos2D

#endif //