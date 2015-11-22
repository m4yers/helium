.text
.globl main
__main__get_five:
  addi  $sp, $sp, -24
  sw    $ra, 20($sp)
  sw    $fp, 16($sp)
  addi  $fp, $sp, 24
  sw    $a0, 0($fp)
  addi  $v0, $zero, 5
  j     L0
L0:
  lw    $ra, 20($sp)
  lw    $fp, 16($sp)
  addi  $sp, $sp, 24
  jr    $ra
__main__sum:
  addi  $sp, $sp, -24
  sw    $ra, 20($sp)
  sw    $fp, 16($sp)
  addi  $fp, $sp, 24
  sw    $a0, 0($fp)
  add   $a1, $a1, $a2
  add   $a1, $a1, $zero
  lw    $a0, 0($fp)
  add   $a0, $a0, $zero
  jal   __main__get_five
  add   $v0, $v0, $zero
  add   $v0, $a1, $v0
  add   $v0, $v0, $zero
  j     L1
L1:
  lw    $ra, 20($sp)
  lw    $fp, 16($sp)
  addi  $sp, $sp, 24
  jr    $ra
main:
  addi  $sp, $sp, -24
  sw    $ra, 20($sp)
  sw    $fp, 16($sp)
  addi  $fp, $sp, 24
  sw    $a0, 0($fp)
  add   $a0, $fp, $zero
  addi  $a1, $zero, 3
  add   $a1, $a1, $zero
  addi  $a2, $zero, 4
  add   $a2, $a2, $zero
  jal   __main__sum
  add   $v0, $v0, $zero
  j     L2
L2:
  lw    $ra, 20($sp)
  lw    $fp, 16($sp)
  addi  $sp, $sp, 24
  jr    $ra
