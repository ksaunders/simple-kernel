	.global _main
_main:
	li	sp, 0x80200000
	call	kmain
1:	wfi
	j	1b
