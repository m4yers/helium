.text
.globl main
main:
  addi  $sp, $sp, -48
  sw    $ra, 20($sp)
  sw    $fp, 16($sp)
  addi  $fp, $sp, 48
  addi  $v0, $fp, -20
  sw    $a0, 0($fp)
  addi  $a0, $zero, 1
  sw    $a0, 0($v0)
  addi  $a0, $zero, 2
  sw    $a0, 4($v0)
  addi  $a0, $zero, 1337
  sw    $a0, 8($v0)
  addi  $a0, $zero, 4
  sw    $a0, 12($v0)
  addi  $a0, $zero, 5
  sw    $a0, 16($v0)
  add   $v0, $v0, $zero
  addi  $a1, $zero, 2
  addi  $a0, $zero, 4
  mult  $a1, $a0
  mflo  $a0
  add   $v0, $v0, $a0
  lw    $v0, 0($v0)
  add   $v0, $v0, $zero
  j     L0
L0:
  lw    $ra, 20($sp)
  lw    $fp, 16($sp)
  addi  $sp, $sp, 48
  jr    $ra
