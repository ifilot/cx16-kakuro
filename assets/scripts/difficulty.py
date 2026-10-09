"""Render menu difficulty from the same PNG source used by the game atlas."""
from functools import lru_cache
from pathlib import Path
from PIL import Image

@lru_cache(maxsize=1)
def source():
    return Image.open(Path(__file__).resolve().parents[1]/'tiles/difficulty.png').convert('RGBA')

def blossom(image,x,y,filled,colors,mini=False,face=None):
    if mini:
        mask=source().crop((32,0,37,5)).getchannel('A')
        image.paste(face if face is not None else colors[1],(x,y),mask)
    else:
        left=1 if filled else 17
        sprite=source().crop((left,1,left+13,14))
        image.paste(sprite,(x,y),sprite.getchannel('A'))
