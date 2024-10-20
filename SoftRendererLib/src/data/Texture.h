#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>
#include "PixelFormat/PixelFormat.h"

namespace Renderer2D{

class Texture
{
public:
    Texture(uint16_t width, uint16_t height, PixelFormat format);
    Texture(uint16_t width, uint16_t height, uint8_t* data, PixelFormat format);
    ~Texture();

    /// @brief Get Pointer of Texture
    /// @return uint8_t*
    uint8_t* GetData();

    
    /// @brief Get Format of Texture
    /// @return PixelFormat
    PixelFormat GetFormat();

    
    uint16_t GetWidth();
    uint16_t GetHeight();
    
private:
    uint8_t* data;
    PixelFormat format;
    bool storedLocally = false;
    uint16_t width, height;
};

}

#endif