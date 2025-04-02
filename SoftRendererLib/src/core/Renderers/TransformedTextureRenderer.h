#ifndef TRANSFORMEDTEXTURERENDERER_H
#define TRANSFORMEDTEXTURERENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"
#include "../../data/Texture.h"
#include <functional>

namespace Tergos2D
{

    using DrawTexturePointer = void (*)(Texture &texture,  const float transformationMatrix[3][3], RenderContext2D& context);


    class TransformedTextureRenderer : RendererBase
    {
    public:
        TransformedTextureRenderer(RenderContext2D &context);
        ~TransformedTextureRenderer() = default;


        void DrawTexture(Texture &texture,  const float transformationMatrix[3][3]);

        /// @brief generic implementation without sampling support
        /// @param texture
        /// @param transformationMatrix
        /// @param context
        static void DrawTexture(Texture &texture,  const float transformationMatrix[3][3], RenderContext2D& context);


        /// @brief  Implementation with Sampling Support supports any BytePixel format (RGBA32,RGB24,Grayscale8, etc ..) not formats like rgb565
        /// @param texture
        /// @param transformationMatrix
        /// @param context
        static void DrawTextureSamplingSupp(Texture &texture,  const float transformationMatrix[3][3], RenderContext2D& context);

        DrawTexturePointer GetDrawTexture();
        void SetDrawTexture(DrawTexturePointer drawTexture);
    private:

        	DrawTexturePointer m_drawTexture = DrawTexture;
    };

} // namespace Tergos2D

#endif //