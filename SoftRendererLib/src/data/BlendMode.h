namespace Renderer2D
{
    enum class BlendMode
    {
        NOBLEND,
        MULTIPLY,
        ADDITIVE,
        ALPHA,      // Blends based on the alpha channel
        SCREEN,     // Brightens the image
        OVERLAY,    // Combines multiply and screen effects
        SUBTRACT,   // Subtracts the source from the destination
        DODGE,      // Lightens the destination based on the source
        BURN        // Darkens the destination based on the source
    };
}
