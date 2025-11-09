# NanoShell
SDK_INC_DIR=../../crt/include
SDK_LIB_DIR=../../crt/

SUPPRESSED_WARNINGS = \
	-Wno-return-type \
	-Wno-incompatible-pointer-types-discards-qualifiers

HACKS = \
	-Dftello=ftell \
	-Dgmtime_r=localtime_r \
	-DDISABLE_NET

CFLAGS_PLAT=-I $(SDK_INC_DIR) -DNANOSHELL -ffreestanding -target i686-elf -nostdinc -nostdlib -mno-mmx -mno-sse -mno-sse2 -fno-pie -fno-pic $(SUPPRESSED_WARNINGS) $(HACKS)
LIBS_PLAT=../../crt/lib/libnanoshell.a ../../crt/lib/crt1.o -T i686_link.ld -nostdlib -nostartfiles
SRCS_PLAT=nanoshell_math.c nanoshell_console.c

override CFLAGS = -O3 ${CFLAGS_PLAT}
override LIBS = ${LIBS_PLAT}
override PROGS = tiny386_nosdl

CC=clang
LD=ld

# Original Makefile begins here
Q ?= @
CC ?= gcc
SDL_CONFIG ?= sdl-config
CFLAGS = -O3 -g `${SDL_CONFIG} --cflags` ${CFLAGS_PLAT}
#CFLAGS = -g `sdl-config --cflags`
LIBS = `${SDL_CONFIG} --libs` -lm ${LIBS_PLAT}

SRCS = ini.c i386.c fpu.c i8259.c i8254.c ide.c vga.c i8042.c misc.c fmopl.c adlib.c ne2000.c i8257.c sb16.c pcspk.c
SRCS += pci.c
SRCS += win32.c
SRCS += ${SRCS_PLAT}

# slirp
#SRCS$ += \
#slirp/bootp.c \
#slirp/cksum.c \
#slirp/if.c \
#slirp/ip_icmp.c \
#slirp/ip_input.c \
#slirp/ip_output.c \
#slirp/mbuf.c \
#slirp/misc.c \
#slirp/sbuf.c \
#slirp/slirp.c \
#slirp/socket.c \
#slirp/tcp_input.c \
#slirp/tcp_output.c \
#slirp/tcp_subr.c \
#slirp/tcp_timer.c \
#slirp/cutils.c \
#slirp/udp.c

# OSD
SRCS += osd/microui.c osd/osd.c

OBJS = ${SRCS:.c=.o}

PROGS = tiny386 tiny386_nosdl tiny386_kvm wifikbd initnet

.PHONY: all clean dep
.SUFFIXES: .c
.c.o:
	@/bin/echo -e " \e[1;32mCC\e[0m\t\e[1;37m$<\e[0m \e[1;32m->\e[0m \e[1;37m$@\e[0m"
	${Q}${CC} ${CFLAGS} -c $< -o $@

all: ${PROGS}

win32:
	make -C . CC?=i686-w64-mingw32-gcc CFLAGS_PLAT=-mconsole LIBS_PLAT="-lws2_32 -liphlpapi" tiny386

clean:
	rm -f ${OBJS} .depends ${PROGS}

tiny386: main.c ${OBJS}
	@/bin/echo -e " \e[1;32mCCLD\e[0m\t\e[1;32m->\e[0m \e[1;37m$@\e[0m"
	${Q}${CC} ${CFLAGS} -o $@ $< ${OBJS} ${LIBS}

tiny386_nosdl: main.c ${OBJS}
	@/bin/echo -e " \e[1;32mCCLD\e[0m\t\e[1;32m->\e[0m \e[1;37m$@\e[0m"
	${Q}${CC} -DNOSDL ${CFLAGS} -o $@ $< ${OBJS} ${LIBS}
	${Q}cp $@ tiny386.nse

tiny386_kvm: main.c kvm.c ${OBJS}
	@/bin/echo -e " \e[1;32mCCLD\e[0m\t\e[1;32m->\e[0m \e[1;37m$@\e[0m"
	${Q}${CC} -DUSEKVM ${CFLAGS} -o $@ $< kvm.c ${OBJS} ${LIBS}

wifikbd: wifikbd.c
	@/bin/echo -e " \e[1;32mCCLD\e[0m\t\e[1;32m->\e[0m \e[1;37m$@\e[0m"
	${Q}${CC} ${CFLAGS} -o $@ wifikbd.c ${LIBS}

initnet: initnet.c
	${Q}${CC} -o $@ initnet.c

.depends: ${SRCS}
	@/bin/echo -e " \e[1;32mDEP\e[0m\t\e[1;37m$^$>\e[0m \e[1;32m->\e[0m \e[1;37m$@\e[0m"
	${Q}rm -f $@
	${Q}for i in $^$>; do ${CC} ${CFLAGS} -MT $$(dirname $$i)/$$(basename -s .c $$i).o -MM $$i 2> /dev/null >> $@ || exit 0; done

dep: .depends
-include .depends
