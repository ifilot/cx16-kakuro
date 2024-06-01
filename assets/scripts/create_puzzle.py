def main():
    vals = [
        0,0,0,0,0,0,0,0,0,
        0,9,2,5,1,0,2,9,0,
        0,7,9,8,2,0,1,8,9,
        0,0,7,9,5,8,0,6,8,
        0,9,5,0,0,6,9,7,0,
        0,0,0,6,5,9,8,0,0,
        0,0,4,3,1,0,7,2,9,
        0,6,3,1,2,0,0,1,3,
        0,9,7,8,4,0,0,7,8,
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