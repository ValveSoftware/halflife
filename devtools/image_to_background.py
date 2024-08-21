from PIL import Image
import sys
import os

def convert_to_tiles(input_file, output_folder, tile_size=(256, 256)):
    # Open the image
    img = Image.open(input_file)
    width, height = img.size

    # Create the output folder if it doesn't exist
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    output_img_folder = os.path.join(output_folder, "background")
    if not os.path.exists(output_img_folder):
        os.makedirs(output_img_folder)

    row = 0
    col = 0
    
    prefix = "21_9"

    # background txt file should look like:
    #
    # resolution	800	600
    #
    # resource/background/800_1_a_loading.tga		fit		0	0
    # resource/background/800_1_b_loading.tga		fit		256	0
    # etc

    layout_file = f"resolution\t{width}\t{height}\n\n"

    # Loop through the image and extract tiles
    for j in range(0, height, tile_size[1]):
        row = row + 1
        col = 0

        for i in range(0, width, tile_size[0]):
            # Calculate the boundaries for the tile, making sure it doesn't exceed the image dimensions
            right_bound = min(i + tile_size[0], width)
            bottom_bound = min(j + tile_size[1], height)
            
            box = (i, j, right_bound, bottom_bound)
            tile = img.crop(box)

            tile_char = chr(ord('a')+col)
            target_file = f"{prefix}_{row}_{tile_char}_loading.tga"
            tile_filename = os.path.join(output_img_folder, target_file)
            tile.save(tile_filename)
            print(f"Saved {tile_filename}")

            line = f"resource/background/{target_file}\tfit\t{i}\t{j}\n"
            layout_file += line

            col = col + 1
    
    print("------------------------------------------")
    print(layout_file)
    print("------------------------------------------")
    
    out_txt = os.path.join(output_folder, "BackgroundLayout.txt")
    with open(out_txt, 'w') as f:
        f.write(layout_file)

    out_txt = os.path.join(output_folder, "BackgroundLoadingLayout.txt")
    with open(out_txt, 'w') as f:
        f.write(layout_file)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python image_to_background.py path_to_image (psd/png/tga)")
        sys.exit(1)

    img_path = sys.argv[1]
    img_dir = os.path.dirname(img_path)
    img_dir = os.path.join(img_dir, "Background")
    
    print(f"out dir: {img_dir}")

    convert_to_tiles(img_path, img_dir)

    #width, height, palette, data = read_spr(spr_path)
    #save_as_png(spr_path + "__.png", width, height, palette, data)

    print(f"Converted {img_path}")
    