CC=color-gcc
CFLAGS=-std=c99 -g -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE -Wall -pedantic
LFLAGS=-lrt

%: %.c
	${CC} ${CFLAGS} -o $@ $< -lncurses -lrt -pthread


clean:
	@-rm -rf o_async_edp 
	@-rm -rf aio_edp
	@-rm -rf aio_edp2
	@-rm -rf yield_switch
	@-rm -rf yield_jmp
	@-rm -rf yield_jmp2
	@-rm -rf unwind_stack
	@-rm -rf switch_ctx
	@-rm -rf switch_ctx2
	@-rm -rf thread_key_barriers
	@echo '[+] cleaned!'
