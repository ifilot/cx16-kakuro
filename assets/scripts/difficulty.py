"""Five-petal difficulty marks inspired by the DOS Kakuro blossom renderer.

Native 13x13 silhouette, adapted to the CX16's paper/rose/brown palette.
"""
from PIL import ImageDraw


def blossom(image,x,y,filled,colors,mini=False,face=None):
    paper,rose,brown,ink=colors
    if mini:
        # Five petals at card scale; a single foreground color stays legible
        # on paper, rose and selected brown cards.
        color=face if face is not None else rose
        rows=('00100','11111','01110','11011','01010')
        draw=ImageDraw.Draw(image)
        for row,line in enumerate(rows):
            for col,pixel in enumerate(line):
                if pixel=='1':draw.point((x+col,y+row),fill=color)
        return
    centers=((6,2),(2,5),(10,5),(4,10),(8,10))
    def inside(px,py):
        return any((px-cx)**2+(py-cy)**2<=6 for cx,cy in centers) or (px-6)**2+(py-6)**2<=9
    draw=ImageDraw.Draw(image)
    for row in range(13):
        for col in range(13):
            if not inside(col,row):continue
            edge=any(not inside(col+dx,row+dy) for dx,dy in ((-1,0),(1,0),(0,-1),(0,1)))
            if filled or edge:draw.point((x+col,y+row),fill=(brown if edge else rose) if filled else rose)
    if filled:
        for dx,dy in ((6,5),(5,6),(7,6),(6,7)):
            draw.point((x+dx,y+dy),fill=paper)
