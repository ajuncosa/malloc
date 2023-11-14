PROJDIR		:= $(realpath $CURDIR))
BUILDDIR	:= $(PROJDIR)/build

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME		:= libft_malloc_$(HOSTTYPE).so

