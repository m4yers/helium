.text
.globl main
main:
  addi  $sp, $sp, -32
  sw    $ra, 20($sp)
  sw    $fp, 16($sp)
  addi  $fp, $sp, 32
  addi  $v0, $fp, -8
  sw    $a0, 0($fp)
  addi  $a0, $zero, 3
  sw    $a0, 0($v0)
  addi  $a0, $zero, 5
  sw    $a0, 4($v0)
  add   $v0, $v0, $zero
  lw    $v0, 4($v0)
  j     L0
L0:
  lw    $ra, 20($sp)
  lw    $fp, 16($sp)
  addi  $sp, $sp, 32
  jr    $ra
