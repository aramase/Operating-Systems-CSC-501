/* pfintr.S - pfintr */


.text
pferrcode: .long 0
.globl  pfintr,pferrcode
pfintr:
popl pferrcode     //store error code
pushfl
cli                // disable interrupt
pushal             //save gr
call pfint         //page fault ISR
popal              // switch back to general regs  
popfl
iret

