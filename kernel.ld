ENTRY(_main)

SECTIONS {
	. = 0x80200000;
	.text : {
		*(.text)
	}
}
