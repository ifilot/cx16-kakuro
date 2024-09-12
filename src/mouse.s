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

.export _init_mouse

.code

;
; Initialize mouse routines
; 
;
.proc _init_mouse: near
    sec
    jsr X16::Kernal::SCREEN_MODE
    lda #1
    jsr X16::Kernal::MOUSE_CONFIG
    rts
.endproc