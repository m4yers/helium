.text
.globl __global__main
main:
  add   $fp, $sp, $zero
  addi  $sp, $sp, -20
  sw    $ra, -4($fp)
  sw    $a0, 0($fp)
  addi  $v0, $zero, 1
  addi  $a0, $zero, 4
  slti  $a0, $a0, 6
  bne   $a0, $zero, L3
L4:
  addi  $v0, $zero, 0
L3:
  beq   $v0, $zero, L2
L1:
  addi  $v0, $zero, 6
  addi  $v0, $v0, 4
L2:
  addi  $v0, $zero, 0x1CE
  j     L5
L5:
  lw    $ra, -4($fp)
  add   $sp, $fp, $zero
  jr    $ra
