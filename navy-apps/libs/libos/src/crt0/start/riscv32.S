.globl  _start
_start:
  mv sp, a0
  move s0, zero
  jal call_main
