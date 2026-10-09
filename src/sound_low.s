;
;                                                                       
;   Author: Ivo Filot <ivo@ivofilot.nl>                                 
;                                                                       
;   CX16-KAKURO is free software:                                      
;   you can redistribute it and/or modify it under the terms of the     
;   GNU General Public License as published by the Free Software        
;   Foundation, either version 3 of the License, or (at your option)    
;   any later version.                                                  
;                                                                       
;   CX16-KAKURO is distributed in the hope that it will be useful,     
;   but WITHOUT ANY WARRANTY; without even the implied warranty         
;   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.             
;   See the GNU General Public License for more details.                
;                                                                       
;   You should have received a copy of the GNU General Public License   
;   along with this program.  If not, see http://www.gnu.org/licenses/. 
;                                                                       
;

.include "x16.inc"

.scope zsmkit
.include "zsmkit.inc"
.endscope

.export _init_sound, _start_bgmusic, _stop_bgmusic
.export _sound_fill_buffers_asm, _play_sfx, _play_music

; Bank 4 upper half is reserved for effects. Legacy color-swap scratch uses
; only $A000-$AFFF; journal caches begin at bank 7.
SFX_BANK = 4
SFX_ADDRESS = $B000
.bss
saved_bank: .res 1
.code
.proc _init_sound: near
    lda #1
    jsr zsmkit::zsm_init_engine
    jsr zsmkit::zsmkit_setisr
    ldx #0
    lda #0
    jsr zsmkit::zsm_setatten
    ldx #1
    lda #0
    jsr zsmkit::zsm_setatten
    rts
.endproc
.proc _start_bgmusic: near
    ldx #0
    jmp zsmkit::zsm_play
.endproc
.proc _stop_bgmusic: near
    ldx #0
    jmp zsmkit::zsm_stop
.endproc
.proc _sound_fill_buffers_asm: near
    jmp zsmkit::zsm_fill_buffers
.endproc
; A/X = filename. Replacing priority 0 closes its previous file.
.proc _play_music: near
    pha
    txa
    tay
    pla
    ldx #0
    jsr zsmkit::zsm_setfile
    ldx #0
    sec
    jsr zsmkit::zsm_setloop
    ldx #0
    jmp zsmkit::zsm_play
.endproc
; A/X = generated effect offset. Priority 1 is a non-looping memory stream.
.proc _play_sfx: near
    clc
    adc #<SFX_ADDRESS
    pha
    txa
    adc #>SFX_ADDRESS
    tay
    lda $00
    sta saved_bank
    lda #SFX_BANK
    sta $00
    pla
    ldx #1
    jsr zsmkit::zsm_setmem
    ldx #1
    clc
    jsr zsmkit::zsm_setloop
    ldx #1
    jsr zsmkit::zsm_play
    lda saved_bank
    sta $00
    rts
.endproc
