LIBSSH2_VERSION ?= 1.11.1
LIBSSH2_URL ?= https://www.libssh2.org/download/libssh2-$(LIBSSH2_VERSION).tar.gz
TARGET ?= m68k-amigaos

PREFIX ?= build/$(TARGET)/prefix
BUILD_DIR ?= build/$(TARGET)
DOWNLOAD_DIR := $(BUILD_DIR)/download
OBJ_DIR := $(BUILD_DIR)/obj

ifdef LIBSSH2_SOURCE_DIR
SRC_DIR := $(LIBSSH2_SOURCE_DIR)
SRC_STAMP :=
else
SRC_ROOT := $(BUILD_DIR)/src
SRC_DIR := $(SRC_ROOT)/libssh2-$(LIBSSH2_VERSION)
SRC_STAMP := $(SRC_DIR)/.unpacked
endif

ifeq ($(origin CC),default)
CC := $(TARGET)-gcc
endif
AR ?= $(TARGET)-ar
RANLIB ?= $(TARGET)-ranlib

TARGET_CFLAGS ?= --std=gnu99 -O2
TARGET_CPPFLAGS ?=

LIBSSH2_CPPFLAGS := \
	-I$(SRC_DIR)/include \
	-I$(SRC_DIR)/src \
	-DLIBSSH2_OPENSSL \
	-DLIBSSH2_DH_GEX_NEW \
	-DLIBSSH2_HAVE_ZLIB \
	-DHAVE_SYS_UIO_H=1 \
	-DHAVE_SYS_SOCKET_H=1

LIBSSH2_SRCS := \
	agent.c \
	bcrypt_pbkdf.c \
	blowfish.c \
	chacha.c \
	channel.c \
	cipher-chachapoly.c \
	comp.c \
	crypt.c \
	crypto.c \
	global.c \
	hostkey.c \
	keepalive.c \
	kex.c \
	knownhost.c \
	libgcrypt.c \
	mac.c \
	mbedtls.c \
	misc.c \
	openssl.c \
	packet.c \
	pem.c \
	poly1305.c \
	publickey.c \
	scp.c \
	session.c \
	sftp.c \
	transport.c \
	userauth.c \
	userauth_kbd_packet.c \
	version.c \
	wincng.c

LIBSSH2_OBJS := $(addprefix $(OBJ_DIR)/,$(LIBSSH2_SRCS:.c=.o))
LIBSSH2_LIB := $(PREFIX)/lib/libssh2.a

.PHONY: all clean cleanall

all: $(LIBSSH2_LIB)

$(LIBSSH2_LIB): $(LIBSSH2_OBJS)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	$(AR) -crs $@ $(LIBSSH2_OBJS)
	$(RANLIB) $@
	cp $(SRC_DIR)/include/libssh2.h $(PREFIX)/include/
	cp $(SRC_DIR)/include/libssh2_sftp.h $(PREFIX)/include/

$(OBJ_DIR)/%.o: $(SRC_STAMP)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(TARGET_CFLAGS) $(LIBSSH2_CPPFLAGS) $(TARGET_CPPFLAGS) -c $(SRC_DIR)/src/$*.c -o $@

$(SRC_DIR)/.unpacked: $(DOWNLOAD_DIR)/libssh2-$(LIBSSH2_VERSION).tar.gz
	@mkdir -p $(SRC_ROOT)
	tar -xzf $< -C $(SRC_ROOT)
	sed -i 's/elapsed_ms = (long)(1000[ *]*difftime(now, start_time));/elapsed_ms = (long)((now - start_time) * 1000);/' $(SRC_DIR)/src/session.c
	patch -d $(SRC_DIR) -p1 < patches/amiga-nonblocking.patch
	touch $@

$(DOWNLOAD_DIR)/libssh2-$(LIBSSH2_VERSION).tar.gz:
	@mkdir -p $(DOWNLOAD_DIR)
	curl -L -o $@ $(LIBSSH2_URL)

clean:
	rm -rf $(OBJ_DIR) $(LIBSSH2_LIB)

cleanall:
	rm -rf $(BUILD_DIR)
