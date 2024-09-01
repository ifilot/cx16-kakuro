;
; Import ZSM routines
;
; MIT License
; 
; Copyright (c) 2023 mooinglemur
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;
; File obtained from: 
; https://github.com/mooinglemur/zsmkit/blob/main/src/zsmkit.inc
;

.import zsm_init_engine
.import zsm_tick
.import zsm_play
.import zsm_stop
.import zsm_rewind
.import zsm_close
.import zsm_fill_buffers
.import zsm_setlfs
.import zsm_setfile
.import zsm_loadpcm
.import zsm_setmem
.import zsm_setatten
.import zsm_setcb
.import zsm_clearcb
.import zsm_getstate
.import zsm_setrate
.import zsm_getrate
.import zsm_setloop
.import zsm_opmatten
.import zsm_psgatten
.import zsm_pcmatten
.import zsm_set_int_rate

.import zcm_setmem
.import zcm_play
.import zcm_stop
.import zsmkit_setisr
.import zsmkit_clearisr