#ifndef PIXELFORMATINFO_H
#define PIXELFORMATINFO_H

#include "PixelFormat.h"
#include <unordered_map>
#include <cstdint>
#include "../Color.h"

namespace Renderer2D
{

    struct PixelFormatInfo
    {
        PixelFormat format;    // The pixel format
        uint8_t bytesPerPixel; // Number of bytes per pixel
        uint8_t numChannels;   // Number of color channels
        bool hasAlpha;         // Whether the format includes an alpha channel
        const char *name;      // A human-readable name for the format

        // Bit masks and shifts for each channel
        uint16_t redMask, greenMask, blueMask, alphaMask;
        uint8_t redShift, greenShift, blueShift, alphaShift;

        PixelFormatInfo() = default;

        PixelFormatInfo(PixelFormat format, int bpp, int channels, bool alpha, const char *name,
                        uint16_t redMask, uint8_t redShift,
                        uint16_t greenMask, uint8_t greenShift,
                        uint16_t blueMask, uint8_t blueShift,
                        uint16_t alphaMask = 0, uint8_t alphaShift = 0)
            : format(format), bytesPerPixel(bpp), numChannels(channels), hasAlpha(alpha), name(name),
              redMask(redMask), redShift(redShift),
              greenMask(greenMask), greenShift(greenShift),
              blueMask(blueMask), blueShift(blueShift),
              alphaMask(alphaMask), alphaShift(alphaShift) {}

        // Convert a normalized 0-255 color back to the original pixel format
        void ConvertFromColor(const Color inColor, uint8_t *dest) const
        {
            // Scale the normalized 0-255 values back to their respective bit depths
            uint8_t r = ScaleToBits(inColor.data[1], redMask);                    // Red channel
            uint8_t g = ScaleToBits(inColor.data[2], greenMask);                  // Green channel
            uint8_t b = ScaleToBits(inColor.data[3], blueMask);                   // Blue channel
            uint8_t a = hasAlpha ? ScaleToBits(inColor.data[0], alphaMask) : 255; // Alpha channel (if present)

            // Pack the color components back into the pixel
            uint32_t pixelData = 0;
            pixelData |= (r << redShift) & redMask;
            pixelData |= (g << greenShift) & greenMask;
            pixelData |= (b << blueShift) & blueMask;
            if (hasAlpha)
            {
                pixelData |= (a << alphaShift) & alphaMask;
            }

            // Store the packed pixel data in the destination array
            if (bytesPerPixel == 1)
            {
                *dest = static_cast<uint8_t>(pixelData & 0xFF); // For 1-byte formats (e.g., indexed colors)
            }
            else if (bytesPerPixel == 2)
            {
                *reinterpret_cast<uint16_t *>(dest) = static_cast<uint16_t>(pixelData & 0xFFFF); // For 16-bit formats
            }
            else if (bytesPerPixel == 3)
            {
                // Handle 24-bit formats (e.g., RGB24) by splitting the packed pixel into individual bytes
                dest[0] = static_cast<uint8_t>((pixelData >> 16) & 0xFF); // Red channel
                dest[1] = static_cast<uint8_t>((pixelData >> 8) & 0xFF);  // Green channel
                dest[2] = static_cast<uint8_t>(pixelData & 0xFF);         // Blue channel
            }
            else if (bytesPerPixel == 4)
            {
                *reinterpret_cast<uint32_t *>(dest) = static_cast<uint32_t>(pixelData & 0xFFFFFFFF); // For 32-bit formats
            }
        }

        inline Color ConvertToColor(const uint8_t *src) const
        {
            // Extract the packed pixel data (assuming the pixel format stores the color channels in a packed manner)
            uint32_t pixelData = 0;

            if (bytesPerPixel == 1)
            {
                pixelData = *src; // For formats like RGB565, packed in a single byte or more
            }
            else if (bytesPerPixel == 2)
            {
                pixelData = *reinterpret_cast<const uint16_t *>(src); // Handle 16-bit packed formats like RGB565
            }
            else if (bytesPerPixel == 3)
            {
                // Handle 24-bit formats like RGB24
                pixelData = (src[0] << 16) | (src[1] << 8) | src[2];
            }
            else if (bytesPerPixel == 4)
            {
                pixelData = *reinterpret_cast<const uint32_t *>(src); // Handle 32-bit packed formats like ARGB8888
            }

            // Extract color components using bit masks and shifts
            uint8_t r = (pixelData & redMask) >> redShift;
            uint8_t g = (pixelData & greenMask) >> greenShift;
            uint8_t b = (pixelData & blueMask) >> blueShift;
            uint8_t a = hasAlpha ? ((pixelData & alphaMask) >> alphaShift) : 255;

            // Scale the color channels to the 8-bit range (0-255)
            r = ScaleTo255(r, redMask);
            g = ScaleTo255(g, greenMask);
            b = ScaleTo255(b, blueMask);
            if (hasAlpha)
            {
                a = ScaleTo255(a, alphaMask); // If there's an alpha channel, scale it
            }

            return Color(r, g, b, a);
        }

    private:
        // Helper function to scale a normalized 0-255 value back to the native bit depth
        uint8_t ScaleToBits(uint8_t value, uint16_t mask) const
        {
            // Calculate the number of bits used in the channel (number of 1s in the mask)
            int numBits = 0;
            uint16_t maskCopy = mask;
            while (maskCopy)
            {
                numBits += (maskCopy & 1);
                maskCopy >>= 1;
            }

            // Scale the 0-255 value back to the native bit depth
            uint8_t maxValue = (1 << numBits) - 1;               // Max value for the channel
            return static_cast<uint8_t>(value * maxValue / 255); // Scale back to native bit depth
        }
        // Helper function to scale the channel value to 0-255 range based on bit depth
        uint8_t ScaleTo255(uint8_t value, uint16_t mask) const
        {
            // Calculate the number of bits used in the channel (number of 1s in the mask)
            int numBits = 0;
            while (mask)
            {
                numBits += (mask & 1);
                mask >>= 1;
            }

            // If the channel uses fewer bits, scale the value to fit in the 0-255 range
            if (numBits < 8)
            {
                uint8_t maxValue = (1 << numBits) - 1;               // Max value for the channel
                return static_cast<uint8_t>(value * 255 / maxValue); // Scale to 0-255
            }
            return value; // No scaling if the value is already 8 bits
        }
    };

    class PixelFormatRegistry
    {
    public:
        static const PixelFormatInfo &GetInfo(PixelFormat format)
        {
            static std::unordered_map<PixelFormat, PixelFormatInfo> formatInfoMap = {
                {PixelFormat::RGB24, {PixelFormat::RGB24, 3, 3, false, "RGB24", 0xFF, 0, 0xFF, 8, 0xFF, 16}},
                {PixelFormat::ARGB8888, {PixelFormat::ARGB8888, 4, 4, true, "ARGB8888", 0xFF, 16, 0xFF, 8, 0xFF, 0, 0xFF, 24}},
                {PixelFormat::RGBA8888, {PixelFormat::RGBA8888, 4, 4, true, "RGBA8888", 0xFF, 24, 0xFF, 16, 0xFF, 8, 0xFF, 0}},
                {PixelFormat::RGBA1555, {PixelFormat::RGBA1555, 2, 4, true, "RGBA1555", 0x7C00, 10, 0x03E0, 5, 0x001F, 0, 0x8000, 15}},
                {PixelFormat::GRAYSCALE8, {PixelFormat::GRAYSCALE8, 1, 1, false, "Grayscale8", 0xFF, 0, 0x00, 0, 0x00, 0}},
                {PixelFormat::RGB565, {PixelFormat::RGB565, 2, 3, false, "RGB565", 0xF800, 11, 0x07E0, 5, 0x001F, 0}},
                {PixelFormat::RGBA4444, {PixelFormat::RGBA4444, 2, 4, true, "RGBA4444", 0xF000, 12, 0x0F00, 8, 0x00F0, 4, 0x000F, 0}}};

            return formatInfoMap.at(format);
        }
    };

}

#endif // PIXELFORMATINFO_H
