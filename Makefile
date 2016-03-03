#------------------------------------------------------------------------------
# Build Configuration
#------------------------------------------------------------------------------
# name and version
VERSION  = 0.0.1

# tools
CC = gcc
LD = ${CC}

# flags
LIBS     = -lexpat -lssl -lresolv
CPPFLAGS = -DVERSION="$(VERSION)"
CFLAGS   = -std=gnu11 ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}
INCS     = -Iinclude/ -Isource/ -I/usr/include/

# commands
COMPILE = ${CC} ${CFLAGS} -c -o $@ $<
LINK    = ${LD} -o $@ $^ ${LDFLAGS}
CLEAN   = @rm -f

#------------------------------------------------------------------------------
# Build-Specific Macros
#------------------------------------------------------------------------------
# library macros
BIN  = acc
SRCS = source/main.c \
       source/auth.c \
       source/conn.c \
       source/ctx.c \
       source/event.c \
       source/handler.c \
       source/hash.c \
       source/jid.c \
       source/md5.c \
       source/parser_expat.c \
       source/rand.c \
       source/resolver.c \
       source/sasl.c \
       source/scram.c \
       source/sha1.c \
       source/snprintf.c \
       source/sock.c \
       source/stanza.c \
       source/thread.c \
       source/util.c \
       source/tls_openssl.c \
       source/uuid.c

# load user-specific settings
-include config.mk

#------------------------------------------------------------------------------
# Phony Targets
#------------------------------------------------------------------------------
.PHONY: all

all: ${BIN}

${BIN}: ${SRCS:.c=.o}
	${LINK}

clean:
	${CLEAN} ${BIN} ${SRCS:.c=.o}
	${CLEAN} ${SRCS:.c=.gcno} ${SRCS:.c=.gcda}

.c.o:
	${COMPILE}

# load dependency files
-include ${DEPS}

