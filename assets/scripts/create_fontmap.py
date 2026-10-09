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

from pathlib import Path
import sys
import numpy as np
from PIL import Image

def main():
    output=Path(sys.argv[1]) if len(sys.argv)>1 else Path('.')
    source=Path(__file__).resolve().parents[1]/'tiles/ui-charmap.png'
    with Image.open(source) as image:
        if image.size!=(128,48):raise ValueError('UI charmap must be 16x6 cells of 8x8 pixels (ASCII 32-127)')
        mask=np.asarray(image.convert('RGBA').getchannel('A'))>150
    data=bytearray()
    for row in range(6):
        for col in range(16):
            data.extend(np.packbits(mask[row*8:row*8+8,col*8:col*8+8],axis=1).tobytes())
    output.mkdir(parents=True,exist_ok=True)
    (output/'FONT8.DAT').write_bytes(data)

if __name__=='__main__':main()
