CC=color-gcc
CFLAGS=-std=c99 -g -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE -Wall -pedantic
LFLAGS=-lrt -lncurses -pthread

sockets_client: sockets_common.c sockets_client.c 
	@${CC} -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -pedantic -o $@ $^

sockets_server: sockets_common.c sockets_server.c 
	@${CC} -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -Wall -pedantic -o $@ $^

sem_sysv_producer:sem_sysv_common.c sem_sysv_producer.c 
	@${CC} -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE -Wall -pedantic -o $@ $^

sem_sysv_consumer:sem_sysv_common.c sem_sysv_consumer.c 
	@${CC} -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE -Wall -pedantic -o $@ $^

%: %.c
	${CC} ${CFLAGS} -o $@ $< ${LFLAGS}

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
	@-rm -rf pts_snoop
	@-rm -rf showIP sockets_client sockets_server child_signals
	@-rm -rf sem_posix_producer sem_posix_consumer
	@-rm -rf sem_sysv_producer sem_sysv_consumer
	@echo '[+] cleaned!'


############
# in older version
#showip: showIP.c
#	@${CC} -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -pedantic -o showIP $^
#	@echo '[+] showIP compiled'
#
# child_signals: child_signals.c
# 	@${CC} -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE -Wall -pedantic -o $@ $<
# 
# sem_posix_producer: sem_posix_producer.c 
# 	@${CC} -std=c99 -pthread -Wall -pedantic -o $@ $<
# 
# sem_posix_consumer: sem_posix_consumer.c 
# 	@${CC} -std=c99 -pthread -Wall -pedantic -o $@ $<

