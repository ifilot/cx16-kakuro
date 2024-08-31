#########################################################################
#                                                                       #
#   Author: Ivo Filot <ivo@ivofilot.nl>                                 #
#                                                                       #
#   CX16-OTHELLO is free software:                                      #
#   you can redistribute it and/or modify it under the terms of the     #
#   GNU General Public License as published by the Free Software        #
#   Foundation, either version 3 of the License, or (at your option)    #
#   any later version.                                                  #
#                                                                       #
#   CX16-OTHELLO is distributed in the hope that it will be useful,     #
#   but WITHOUT ANY WARRANTY; without even the implied warranty         #
#   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.             #
#   See the GNU General Public License for more details.                #
#                                                                       #
#   You should have received a copy of the GNU General Public License   #
#   along with this program.  If not, see http://www.gnu.org/licenses/. #
#                                                                       #
#########################################################################

import numpy as np
import PIL.Image
import os
import sys

ROOT = os.path.dirname(__file__)

def main():
    """
    Open .png file storing an 16x16 bitmap font, extract the relevant characters
    and store this at a 1bpp fontmap file.
    """
    img = PIL.Image.open(os.path.join(ROOT, '..', 'tiles', 'font-tiles.png'))
    
    data = bytearray()
    for i in range(10):                                 # loop over rows
        for j in range(16):                             # loop over columns
            for y in  range(16):
                # first 8 columns
                b = np.uint8(0x00)
                for k,x in enumerate(range(0,8)):
                    px = img.getpixel((j*16+x, i*16+y))
                    if px[3] > 150:
                        b |= (1 << (8-k-1))
                data.append(b)

                # last 8 columns
                b = np.uint8(0x00)
                for k,x in enumerate(range(8,16)):
                    px = img.getpixel((j*16+x, i*16+y))
                    if px[3] > 150:
                        b |= (1 << (8-k-1))                
                data.append(b)

    with open(sys.argv[1], 'wb') as f:
        f.write(data)

if __name__ == '__main__':
    main()