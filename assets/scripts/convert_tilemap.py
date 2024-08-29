#########################################################################
#                                                                       #
#   Author: Ivo Filot <ivo@ivofilot.nl>                                 #
#                                                                       #
#   CX16-KAKURO is free software:                                       #
#   you can redistribute it and/or modify it under the terms of the     #
#   GNU General Public License as published by the Free Software        #
#   Foundation, either version 3 of the License, or (at your option)    #
#   any later version.                                                  #
#                                                                       #
#   CX16-KAKURO is distributed in the hope that it will be useful,      #
#   but WITHOUT ANY WARRANTY; without even the implied warranty         #
#   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.             #
#   See the GNU General Public License for more details.                #
#                                                                       #
#   You should have received a copy of the GNU General Public License   #
#   along with this program.  If not, see http://www.gnu.org/licenses/. #
#                                                                       #
#########################################################################

import PIL.Image
import os
import numpy as np
import sys

ROOT = os.path.dirname(__file__)

def main():
    """
    Produce a TILEMAP for the CX16 by opening a .png file containing the tiles
    and a .png file containing the palette. Search for the best color in the
    palette matching any pixel and store the tiles in a color-encoded data file
    based on the palette.
    """
    # grab the colors
    palette = get_color_palette()
    
    # encode tiles based on the color palette
    img = PIL.Image.open(os.path.join(ROOT, '..', 'tiles', sys.argv[1]))
    data = build_tiles(img, palette)
    
    with open(sys.argv[2], 'wb') as f:
        f.write(data)

def build_tiles(img, palette):
    """
    Grab the tiles from an image file and compare the colors with a palette.
    Pick the best matching color and store the tilemap using the color-encoding
    as supplied by the palette.
    """
    data = bytearray()
    
    for i in range(6):      # loop over rows
        for j in range(16):  # loop over columns
            
            # loop over pixels
            for y in range(16):
                for x in range(16):
                    col = img.getpixel(((j*16+x),(i*16+y)))
                    idx = find_closest_color(col, palette)
                    data.extend(idx)
    
    return data

def find_closest_color(col, palette):
    """
    Find closest color match from palette given a pixel color
    """
    # if transparent, return 0
    if col[3] < 50:
        return np.uint8(0x00)
    
    distances = np.linalg.norm(palette[1:,:] - col[0:3], axis=1)
    return np.uint8(np.argmin(distances)+1)

def get_color_palette():
    """
    Build the color palette from the CX16 palette png file
    """
    file = os.path.join(ROOT, '..', 'palette', 'cx16palette.png')
    img = PIL.Image.open(file)
    
    colors = []
    for i in range(16):
        for j in range(16):
            col = img.getpixel((j*64+32,i*64+32))
            colors.append(col)
    
    return np.array(colors, np.uint8)
                    
if __name__ == '__main__':
    main()