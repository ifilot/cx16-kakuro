; Fast contiguous transfers and bank-spanning bitmap rectangles.
.export _transfer_clear, _transfer_read, _transfer_write
.export _transfer_count, _transfer_pointer, _rectangle_read, _rectangle_write
.import _menu_blit_source, _menu_blit_width, _menu_blit_height
.segment "ZEROPAGE"
pointer: .res 2
remaining: .res 2
address: .res 3
rows: .res 1
mode: .res 1
.bss
_transfer_count: .res 2
_transfer_pointer: .res 2
.code
.proc setup
    lda _transfer_pointer
    sta pointer
    lda _transfer_pointer+1
    sta pointer+1
    lda _transfer_count
    sta remaining
    lda _transfer_count+1
    sta remaining+1
    rts
.endproc
.proc decrement
    lda remaining
    bne :+
    dec remaining+1
:
    dec remaining
    lda remaining
    ora remaining+1
    rts
.endproc
; All map transfers are whole 256-byte pages. Indexing avoids per-byte
; 16-bit counters and keeps IRQs/music enabled throughout the copy.
.proc _transfer_clear
    ldy _transfer_count+1
page:
    ldx #0
byte:
    stz $9f23
    dex
    bne byte
    dey
    bne page
    rts
.endproc
.proc _transfer_read
    jsr setup
    ldx remaining+1
    ldy #0
byte:
    lda $9f23
    sta (pointer),y
    iny
    bne byte
    inc pointer+1
    dex
    bne byte
    rts
.endproc
.proc _transfer_write
    jsr setup
    ldx remaining+1
    ldy #0
byte:
    lda (pointer),y
    sta $9f23
    iny
    bne byte
    inc pointer+1
    dex
    bne byte
    rts
.endproc
.proc _rectangle_read
    lda #1
    bra rectangle
.endproc
.proc _rectangle_write
    lda #0
    bra rectangle
.endproc
rectangle:
    sta mode
    lda _menu_blit_source
    sta pointer
    lda _menu_blit_source+1
    sta pointer+1
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
    ldx _menu_blit_width
byte:
    lda mode
    beq write
    lda $9f23
    sta (pointer)
    bra next
write:
    lda (pointer)
    sta $9f23
next:
    inc pointer
    bne :+
    inc pointer+1
    lda pointer+1
    cmp #$c0
    bne :+
    lda #$a0
    sta pointer+1
    inc $00
:
    dex
    bne byte
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
