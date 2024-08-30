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

import numpy as np
import os
import struct

ROOT = os.path.dirname(__file__)

def main():
    data = bytearray()
    offsets = []
    offset = 0
    for i in range(48):
        puzdata = encode_puzzle('%03i.puz' % (i+1))
        offsets.append(offset)
        data += puzdata
        offset += len(puzdata)

    print(offsets)

    with open('PUZZLE.DAT', 'wb') as f:
        f.write(struct.pack('<H', len(offsets)))
        for o in offsets:
            f.write(struct.pack('<H', o + (len(offsets) + 1) * 2))
        f.write(data)

def encode_puzzle(filename):
    (rows, cols), values, knowns = read_file(os.path.join(ROOT, '..', 'puzzles', filename))
    nrcells = rows * cols

    # store numeric data
    data = bytearray()
    data.append(rows << 4 | cols)
    data.extend([0] * (nrcells // 2 + nrcells % 2))

    for i in range(rows):
        for j in range(cols):
            idx = i * cols + j
            data[idx // 2 + 1] |= (values[i,j] << (4 if (idx % 2 == 0) else 0))

    # append known positions
    data.append(np.uint8(len(knowns)))
    for k in knowns:
        data.append(k[0] << 4 | k[1])

    return data

def read_file(filename):
    with open(filename, 'r') as f:
        data = []
        rawdata = []
        
        for line in f.readlines():
            if line.startswith('#'):
                continue
            data.append(line.strip().replace('#','').split())
            rawdata.append(line.strip().split())
        
        # set puzzle dimensions
        rows = len(data)
        cols = len(data[0])
        
        # convert to numpy arrays
        values = np.array(data, dtype=np.uint8).reshape(rows, cols)
        
        # capture knowns
        knowns = []
        for j in range(rows):
            for i in range(cols):
                if rawdata[j][i].startswith('#'):
                    knowns.append((j,i))

    return (rows, cols), values, knowns

if __name__ == '__main__':
    main()