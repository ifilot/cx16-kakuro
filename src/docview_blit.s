; Write one document line and its narrow scrollbar without C per-pixel work.
.export _docview_text_row, _docview_bar, _docview_color
.export _docview_thumb_top, _docview_thumb_end
.import _menu_blit_source
.segment "ZEROPAGE"
source: .res 2
address: .res 3
line: .res 2
.bss
_docview_color: .res 1
_docview_thumb_top: .res 2
_docview_thumb_end: .res 2
.code
.proc _docview_text_row
    lda _menu_blit_source
    sta source
    lda _menu_blit_source+1
    sta source+1
    ldy #0
loop:
    lda (source),y
    sec
    sbc #32
    sta $9f23
    lda _docview_color
    sta $9f23
    iny
    cpy #60
    bne loop
    rts
.endproc
.proc _docview_bar
    lda #$90
    sta address
    lda #$46
    sta address+1
    lda #$10
    sta address+2
    stz line
    stz line+1
loop:
    lda address
    sta $9f20
    lda address+1
    sta $9f21
    lda address+2
    sta $9f22
    lda line
    cmp _docview_thumb_top
    lda line+1
    sbc _docview_thumb_top+1
    bcc track
    lda line
    cmp _docview_thumb_end
    lda line+1
    sbc _docview_thumb_end+1
    bcs track
    lda #$aa
    sta $9f23
    sta $9f23
    bra next
track:
    lda #$01
    sta $9f23
    lda #$40
    sta $9f23
next:
    clc
    lda address
    adc #160
    sta address
    bcc :+
    inc address+1
    bne :+
    inc address+2
:
    inc line
    bne :+
    inc line+1
:
    lda line+1
    beq loop
    lda line
    cmp #32
    bne loop
    rts
.endproc
