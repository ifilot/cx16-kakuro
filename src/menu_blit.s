; Copy a cached 2bpp rectangle to the 640-pixel-wide menu bitmap.
; C supplies the source and dimensions, and sets VERA's starting address.
; Records never cross an 8 KiB RAM bank. Interrupts remain enabled.
.export _menu_blit, _menu_blit_source, _menu_blit_width, _menu_blit_height

.segment "ZEROPAGE"
source: .res 2
address: .res 3
rows: .res 1

.bss
_menu_blit_source: .res 2
_menu_blit_width: .res 1
_menu_blit_height: .res 1

.code
.proc _menu_blit
    lda _menu_blit_source
    sta source
    lda _menu_blit_source+1
    sta source+1
    lda _menu_blit_height
    sta rows
    lda $9f20
    sta address
    lda $9f21
    sta address+1
    lda $9f22
    and #1
    ora #$10
    sta address+2
row:
    lda address
    sta $9f20
    lda address+1
    sta $9f21
    lda address+2
    sta $9f22
    ldy #0
byte:
    lda (source),y
    sta $9f23
    iny
    cpy _menu_blit_width
    bne byte
    clc
    tya
    adc source
    sta source
    bcc :+
    inc source+1
:
    clc
    lda address
    adc #160
    sta address
    bcc :+
    inc address+1
    bne :+
    inc address+2
:
    dec rows
    bne row
    rts
.endproc
