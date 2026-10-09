.macpack longbranch
; Read bounded resources with MACPTR directly into banked RAM or VERA DATA0.
; MACPTR may return short reads and increments the RAM bank automatically.
.export _resource_read
.import _resource_pointer, _resource_remaining, _resource_stream
.import _resource_banked, _resource_error
.segment "ZEROPAGE"
destination: .res 2
.bss
received: .res 2
requested: .res 1
.code
.proc _resource_read
next:
    lda _resource_remaining
    ora _resource_remaining+1
    bne :+
    jmp done
:
    lda #0
    ldx _resource_remaining+1
    cpx #2
    jcs request
    lda #255
    cpx #1
    jeq request
    lda _resource_remaining
request:
    sta requested
    ldx _resource_pointer
    ldy _resource_pointer+1
    clc
    lda _resource_stream
    beq :+
    sec
:
    lda requested
    jsr $ff44
    jcs fallback
    stx received
    sty received+1
    txa
    ora received+1
    jeq error
    jsr $ffb7
    and #$bf                 ; EOF with the final byte is not an I/O error.
    jne error
advance:
    sec
    lda _resource_remaining
    sbc received
    sta _resource_remaining
    lda _resource_remaining+1
    sbc received+1
    sta _resource_remaining+1
    jcc error
    lda _resource_stream
    jne next
    clc
    lda _resource_pointer
    adc received
    sta _resource_pointer
    lda _resource_pointer+1
    adc received+1
    sta _resource_pointer+1
    lda _resource_banked
    jeq next
    lda _resource_pointer+1
    cmp #$c0
    bcs :+
    jmp next
:
    sec
    sbc #$20
    sta _resource_pointer+1
    jmp next
fallback:
    lda _resource_pointer
    sta destination
    lda _resource_pointer+1
    sta destination+1
    jsr $ffa5
    sta (destination)
    jsr $ffb7
    and #$bf
    jne error
    lda #1
    sta received
    stz received+1
    ; In the byte fallback, bank rollover has not been handled by MACPTR.
    lda _resource_banked
    jeq advance
    lda _resource_pointer
    cmp #$ff
    jne advance
    lda _resource_pointer+1
    cmp #$bf
    jne advance
    inc $00
    jmp advance
error:
    lda #1
    sta _resource_error
done:
    rts
.endproc
