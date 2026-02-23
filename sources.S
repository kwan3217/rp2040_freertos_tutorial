// sources.S -- include source tarball as read-only data blob in final executable
.section .rodata,"a",%progbits

.globl sources_start
sources_start:
    .incbin "sources.tar.gz"        // Compressed tarball of source code

.globl sources_end
sources_end:

    .size sources_start, sources_end - sources_start

.balign 16