#ifndef BASICTEXTURERENDERER_H
#define BASICTEXTURERENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"
#include "../../data/Texture.h"
#include <functional>
#if ENABLE_ESP_SUPPORT
#include "esp_attr.h"
#endif
namespace Tergos2D
{
    using BlendFunction = std::function<Color(const Color &src, const Color &dst)>;
    class BasicTextureRenderer : RendererBase
    {
    public:
        BasicTextureRenderer(RenderContext2D &context);
        ~BasicTextureRenderer() = default;

        void IRAM_ATTR DrawTexture(Texture &texture, int16_t x, int16_t y);

    private:
    };

} // namespace Tergos2D

#endif //