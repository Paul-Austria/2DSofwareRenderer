#!/usr/bin/env python3

import sys
from PIL import Image

def rgb565_conversion(input_path, output_path):
    try:
        img = Image.open(input_path).convert('RGB')
        width, height = img.size
        rgb565_data = []

        for y in range(height):
            for x in range(width):
                r, g, b = img.getpixel((x, y))
                r = (r >> 3) & 0x1F
                g = (g >> 2) & 0x3F
                b = (b >> 3) & 0x1F
                rgb565 = (r << 11) | (g << 5) | b
                rgb565_data.append(rgb565)

        with open(output_path, 'wb') as f:
            for pixel in rgb565_data:
                f.write(pixel.to_bytes(2, byteorder='big'))

        print(f"Conversion to RGB565 complete. File saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found.")
    except Exception as e:
        print(f"An error occurred during RGB565 conversion: {e}")

def argb8888_conversion(input_path, output_path):
    try:
        img = Image.open(input_path).convert('RGBA')
        width, height = img.size
        argb8888_data = []

        for y in range(height):
            for x in range(width):
                r, g, b, a = img.getpixel((x, y))
                argb8888 = (a << 24) | (r << 16) | (g << 8) | b
                argb8888_data.append(argb8888)

        with open(output_path, 'wb') as f:
            for pixel in argb8888_data:
                f.write(pixel.to_bytes(4, byteorder='big'))

        print(f"Conversion to ARGB8888 complete. File saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found.")
    except Exception as e:
        print(f"An error occurred during ARGB8888 conversion: {e}")

def main():
    if len(sys.argv) != 4:
        print("Usage: convert_image.py <input_image> <output_file> <format>")
        print("Format options: rgb565, argb8888")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    format_option = sys.argv[3].lower()

    if format_option == 'rgb565':
        rgb565_conversion(input_path, output_path)
    elif format_option == 'argb8888':
        argb8888_conversion(input_path, output_path)
    else:
        print("Error: Invalid format. Supported formats are 'rgb565' and 'argb8888'.")
        sys.exit(1)

if __name__ == "__main__":
    main()
