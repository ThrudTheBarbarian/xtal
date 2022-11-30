#Function calling protocol

function preamble
- push y,x,a
- free all registers
- define the function / clobber (in case we use it later)

body
- code
- rts

function postamble
- pop a,x,y

