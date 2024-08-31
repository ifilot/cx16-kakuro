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
    
    # encode tiles based on the color palette
    img = PIL.Image.open(os.path.join(ROOT, '..', 'fonts', 'reffspixelfont_spritesheet_0_to_1023.png'))   
    canvas = PIL.Image.new(mode="RGBA", size=(16*16, 16*6))
    
    for i in range(6):
        for j in range(16):
            idx = i * 16 + j
            x = idx % 32
            y = idx // 32
            canvas.paste(img.crop((x*17,(y+1)*17,x*17+16,(y+1)*17+16)), (j*16,i*16))
    
    canvas.save(os.path.join(ROOT, '..', 'fonts', 'fontmap.png'))
                    
if __name__ == '__main__':
    main()