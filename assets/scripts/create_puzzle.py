import numpy as np

def main():
    vals = [
        0,0,0,0,0,0,0,0,0,
        0,0,0,2,7,0,7,9,8,
        0,9,2,3,0,7,9,8,6,
        0,2,1,0,6,9,0,2,4,
        0,8,6,0,1,5,0,5,9,
        0,7,3,1,2,0,8,0,0,
        0,0,0,3,4,8,9,0,0,
        0,1,8,2,0,5,4,1,2,
        0,4,9,8,0,9,7,3,8
    ] 

    rows = 9
    cols = 9
    nrcells = rows * cols

    data = bytearray()
    data.append(rows << 4 | cols)
    data.extend([0] * (nrcells // 2 + nrcells % 2))
    for i in range(rows):
        for j in range(cols):
            idx = i * cols + j
            data[idx // 2 + 1] |= (vals[idx] << (4 if (idx % 2 == 0) else 0))

    with open('PUZZLE.DAT', 'wb') as f:
        f.write(data)

if __name__ == '__main__':
    main()