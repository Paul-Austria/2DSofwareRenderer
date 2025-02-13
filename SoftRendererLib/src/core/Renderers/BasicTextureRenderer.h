#ifndef BASICTEXTURERENDERER_H
#define BASICTEXTURERENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"
#include "../../data/Texture.h"
#include <functional>

namespace Renderer2D
{
    using BlendFunction = std::function<Color(const Color &src, const Color &dst)>;
    class BasicTextureRenderer : RendererBase
    {
    public:
        BasicTextureRenderer(RenderContext2D &context);
        ~BasicTextureRenderer() = default;

        void DrawTexture(Texture &texture, int16_t x, int16_t y);

    private:
    };

} // namespace Renderer2D

#endif //