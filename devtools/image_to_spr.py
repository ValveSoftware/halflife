import os
import sys
import struct
from PIL import Image
import math


# Sprite type:
# 0 = VP_PARALLEL_UPRIGHT,
# 1 = FACING_UPRIGHT,
# 2 = VP_PARALLEL,
# 3 = ORIENTED,
# 4 = VP_PARALLEL_ORIENTED

# Texture format:
# 0 = SPR_NORMAL
# 1 = SPR_ADDITIVE
# 2 = SPR_INDEXALPHA
# 3 = SPR_ALPHTEST

#TODO: original width / height preservation throughout, use that for offsets and bounding radii

def write_spr_header(spr_file, width, height, numframes, sprite_type=2, texture_format=1, synctype=0):
    bounding_radius = 32
    if width > 0 and height > 0:
        bounding_radius = math.sqrt( ( (width / 2) ** 2 ) + ( ( height / 2) ** 2 ) )

    spr_file.write(b'IDSP')  # Identifier
    spr_file.write(struct.pack('<i', 2))  # Version (2 for Half-Life)
    spr_file.write(struct.pack('<i', sprite_type))  # Sprite type
    spr_file.write(struct.pack('<i', texture_format)) #Texture Format (0 = SPR_NORMAL)
    spr_file.write(struct.pack('<f', bounding_radius))
    spr_file.write(struct.pack('<i', width))  # Width
    spr_file.write(struct.pack('<i', height))  # Height
    spr_file.write(struct.pack('<i', numframes))  # Number of frames
    spr_file.write(struct.pack('<f', 0.0))  # Beamlength (not used)
    spr_file.write(struct.pack('<i', synctype))  # Synctype (0 for synchronized)

def write_palette_data(spr_file, image, palette):
    spr_file.write(struct.pack('<h', 256))
    #palette = image.getpalette()#[:768]  # 256 colors * 3 (R, G, B)
    spr_file.write(bytearray(palette))

def closest_color_index(rgb, color_list):
    """Find the index of the closest color in the list."""
    distances = [euclidean_distance(rgb, color) for color in color_list]
    return distances.index(min(distances))

def euclidean_distance(c1, c2):
    """Calculate the Euclidean distance between two colors."""
    return sum((a - b) ** 2 for a, b in zip(c1, c2)) ** 0.5

def write_indexed_data(spr_file, image, palette):
    img_data = list( image.getdata() )
    indexed_data = []  
    
    palette_colors = []
    for i in range(0, len(palette)-4, 3):
        palette_colors.append( palette[i:i+3] )
    
    img_colors = []
    for i in range(0, len(img_data), 1):
        col = img_data[i]
        img_colors.append( list(col) )
    
    #print(f"palette_colors: {palette_colors}\n\n")
    
    num_prints = 0
    
    for p in img_colors:
        idx = 0
        if p in palette_colors:
            #print(f"found {p} in palette_colors")
            idx = palette_colors.index(p)
        else:
            num_prints = num_prints + 1
            if num_prints < 30:
                print(f"couldn't find {p}")
            
        indexed_data.append(idx)

    spr_file.write(bytearray(indexed_data))

#   for index in indexed_data:
#      spr_file.write(struct.pack('<B', index))

def next_power_of_two(n):
    """Return the next power of two for given n."""
    return 2 ** (n - 1).bit_length()

def pad_image_to_power_of_two(image):
    """Pad image dimensions to the next power of two."""
    width, height = image.size
    new_width = next_power_of_two(width)
    new_height = next_power_of_two(height)
    
    # Create a new blank image with the padded size
    padded_image = Image.new("RGB", (new_width, new_height), 0)
    padded_image.paste(image, (0, 0))
    return padded_image

def create_palette(img):
    """Extract unique colors from the image and create a palette."""
    # Get the list of all colors in the image
    colors = list(img.getdata())
    # Deduplicate the colors
    unique_colors = sorted(list(set(colors)))
    #print( f"unique_colors: {unique_colors}" )
    
    # Create a palette
    palette = []
    for color in unique_colors:
        palette.extend(color[:3])  # We only want RGB, not RGBA

    # Fill the rest of the 256-color palette with black
    while len(palette) < 256 * 3:
        palette.extend((0, 0, 0))

    #print( f"palette: {palette}" )
        
    return unique_colors, palette

def palettize_image(img, unique_colors, palette):
    # Create a palette image whose size does not matter
    arbitrary_size = 16, 16
    palimage = Image.new('P', arbitrary_size)
    palimage.putpalette(palette)    
    
    img_p = img.convert("P", 0, palimage.im)
    img_p.putpalette(palette)
    return img_p

def img_to_spr(img_path, spr_path):
    # 1. Read the image file and convert to indexed color
    image = Image.open(img_path).convert("RGBA")
    
    # 1.1. Pad image to power of two dimensions
    image = pad_image_to_power_of_two(image)
    width, height = image.size

    # Extract unique colors and create a palette
    unique_colors, palette = create_palette(image)

    # Palettize the image
    # image.save(spr_path + "_PALETTETIME.png")

    #print("\n\n\n")

    with open(spr_path, 'wb') as spr_file:
        # 2. Write SPR header
        write_spr_header(spr_file, width, height, 1)

        # 3. Write the palette data
        write_palette_data(spr_file, image, palette)
        
        # 4. Write the frame header
        spr_file.write(struct.pack('<i', 0)) # frame group
        spr_file.write(struct.pack('<i', int(-width/2)) ) # frame_origin_x
        spr_file.write(struct.pack('<i', int(-height/2)) ) # frame_origin_y
        spr_file.write(struct.pack('<i', width)) # frame_width
        spr_file.write(struct.pack('<i', height)) # frame_height
        
        # 4.1 Write the indexed data
        write_indexed_data(spr_file, image, palette)
        
def read_spr(filename):
    with open(filename, 'rb') as f:
        # Read the SPR header
        id = f.read(4).decode()
        if id != 'IDSP':
            raise ValueError("Not a valid SPR file")

        version, = struct.unpack('<i', f.read(4))
        type, = struct.unpack('<i', f.read(4))

        if type != 2:  # 2 = VP_PARALLEL
            raise ValueError(f"Only VP_PARALLEL type supported, got {type}")

        # other header info... 
        format, = struct.unpack('<i', f.read(4))        
        bounding_radius, = struct.unpack('<f', f.read(4))
        width, = struct.unpack('<i', f.read(4))
        height, = struct.unpack('<i', f.read(4))
        num_frames, = struct.unpack('<i', f.read(4))
        beam_len, = struct.unpack('<f', f.read(4))
        sync_type, = struct.unpack('<i', f.read(4))

        #print(f"format: {format}\nbounding_radius: {bounding_radius}\nwidth: {width}\nheight: {height}\nnum_frames: {num_frames}\nbeam_len: {beam_len}\nsync_type: {sync_type}\n")

        # For simplicity, assume a single frame
        if num_frames != 1:
            raise ValueError(f"Only single frame SPRs supported, got {num_frames}")

        palette_len, = struct.unpack('<h', f.read(2))

        # Read the palette (256 RGB entries)
        palette = [(ord(f.read(1)), ord(f.read(1)), ord(f.read(1))) for _ in range(256)]
        #print( f"palette: {palette}" )

        # Read the frame
        frame_group, = struct.unpack('<i', f.read(4))
        frame_origin_x, = struct.unpack('<i', f.read(4))
        frame_origin_y, = struct.unpack('<i', f.read(4))
        frame_width, = struct.unpack('<i', f.read(4))
        frame_height, = struct.unpack('<i', f.read(4))
        
        #print(f"frame_group: {frame_group} frame_origin_x: {frame_origin_x} frame_origin_y: {frame_origin_y} frame_width: {frame_width} frame_height: {frame_height}")
        
        data = f.read(width * height)

    return width, height, palette, data

def save_as_png(filename, width, height, palette, data):
    img_data = [palette[byte] for byte in data]
    img = Image.new('RGB', (width, height))
    img.putdata(img_data)
    img.save(filename)
        

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python image_to_spr.py path_to_image (psd/png)")
        sys.exit(1)

    img_path = sys.argv[1]
    spr_path = img_path.rsplit('.', 1)[0] + ".spr"  # Replace .png with .spr for output
    print(f"{img_path} -> {spr_path}")
    img_to_spr(img_path, spr_path)

    width, height, palette, data = read_spr(spr_path)
    save_as_png(spr_path + "__.png", width, height, palette, data)

    print(f"Converted {img_path} to {spr_path}")
    
    #spr_path = 'content/materialsrc/Sprites/1280/test.spr'
    #width, height, palette, data = read_spr(spr_path)
    #save_as_png(spr_path + "__.png", width, height, palette, data)
    