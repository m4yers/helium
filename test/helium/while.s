.text
.globl main
main:
  add   $fp, $sp, $zero
  addi  $sp, $sp, -20
  sw    $ra, -4($fp)
  sw    $a0, 0($fp)
  addi  $v0, $zero, 10
L1:
  addi  $a0, $zero, 1
  addi  $a1, $zero, 1
  slt   $a1, $a1, $v0
  bne   $a1, $zero, L3
L4:
  addi  $a0, $zero, 0
L3:
  beq   $a0, $zero, L0
L2:
  addi  $v0, $v0, -1
  j     L1
L0:
  addi  $v0, $zero, 0x701D
  j     L5
L5:
  lw    $ra, -4($fp)
  add   $sp, $fp, $zero
  jr    $ra
