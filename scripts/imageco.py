from PIL import Image

def rgb565_conversion(image_path, output_path):
    img = Image.open(image_path)
    img = img.convert('RGB')  # Ensure the image is in RGB mode
    width, height = img.size
    rgb565_img = []

    for y in range(height):
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            r = (r >> 3) & 0x1F
            g = (g >> 2) & 0x3F
            b = (b >> 3) & 0x1F
            rgb565 = (r << 11) | (g << 5) | b
            rgb565_img.append(rgb565)

    # Write the converted image to a binary file
    with open(output_path, 'wb') as f:
        for pixel in rgb565_img:
            f.write(pixel.to_bytes(2, byteorder='big'))

# Example usage
rgb565_conversion('img.png', 'output.rgb')
