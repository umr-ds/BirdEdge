################################################################################
# Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
################################################################################

CUDA_VER:=10

APP:= birdedge

TARGET_DEVICE = $(shell gcc -dumpmachine | cut -f1 -d -)


NVDS_VERSION:=6.0

DS_PATH := /opt/nvidia/deepstream/deepstream-$(NVDS_VERSION)/sources

BUILD_DIR := build

LIB_INSTALL_DIR?=/opt/nvidia/deepstream/deepstream-$(NVDS_VERSION)/lib/


ifeq ($(TARGET_DEVICE),aarch64)
  CFLAGS:= -DPLATFORM_TEGRA
endif

SRCS:= $(wildcard *.c)
SRCS+= $(wildcard ./apps-common/src/*.c)

INCS:= $(wildcard *.h)
#INCS+= $(wildcard ./apps-common/includes/*.h)


PKGS:= gstreamer-1.0 json-glib-1.0 gstreamer-video-1.0

# OBJS:= $(SRCS:.c=.o)
OBJS := $(patsubst %,$(BUILD_DIR)/%,$(SRCS:.c=.o))

CFLAGS+= -DDS_VERSION_MINOR=0 -DDS_VERSION_MAJOR=5 -I./apps-common/includes/ -I$(DS_PATH)/includes/
CFLAGS+= -I /usr/local/cuda-$(CUDA_VER)/include

LIBS:= -L/usr/local/cuda-$(CUDA_VER)/lib64/ -lcudart

LIBS+= -L$(LIB_INSTALL_DIR) -Wl,-rpath,$(LIB_INSTALL_DIR) -lgstrtspserver-1.0 -ldl -lcuda
LIBS+= -lnvdsgst_meta -lnvds_meta -lnvdsgst_helper -lnvdsgst_smartrecord -lnvds_utils -lnvds_msgbroker -lm

CFLAGS+= `pkg-config --cflags $(PKGS)`

LIBS+= `pkg-config --libs $(PKGS)`

all: $(APP)

$(BUILD_DIR)/%.o: %.c $(INCS) Makefile
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $<

$(APP): $(OBJS) Makefile
	$(CC) -o $(APP) $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) $(APP)
