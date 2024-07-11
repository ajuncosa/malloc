PROJDIR			:= $(realpath $(CURDIR))
SRCSDIR			:= $(PROJDIR)/src
TESTDIR			:= $(PROJDIR)/test
INCLUDESDIR		:= $(PROJDIR)/includes
BUILDDIR		:= $(PROJDIR)/build
OBJSDIR			:= $(BUILDDIR)/objs
LIBDIR			:= $(BUILDDIR)/lib
BINDIR			:= $(BUILDDIR)/bin
LIBFTDIR		:= ${SRCSDIR}/libft

VERBOSE			:= FALSE

SRCS_LIST		:= malloc.c heap.c
OBJS_LIST		:= $(SRCS_LIST:.c=.o)
TEST_COMMON		:= $(TESTDIR)/test_utils.c
TEST_SRCS_LIST	:= init.c \
				   tiny_malloc_1.c \
				   tiny_malloc_2.c \
				   tiny_malloc_3.c \
				   tiny_malloc_4.c \
				   small_malloc_1.c \
				   small_malloc_2.c \
				   small_malloc_3.c \
				   small_malloc_4.c
TEST_BINS_LIST	:= $(TEST_SRCS_LIST:.c=)

OBJS			:= $(addprefix $(OBJSDIR)/,$(OBJS_LIST))
TEST_BINS		:= $(addprefix $(BINDIR)/test_,$(TEST_BINS_LIST))

LIBFT		:= $(LIBDIR)/libft.a

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif
LIBRARY_NAME := ft_malloc_$(HOSTTYPE)

NAME		:= $(LIBDIR)/lib$(LIBRARY_NAME).so

CC			:= gcc
RM			:= rm -rf
COMMON		= -Wall -Werror -Wextra
CFLAGS		?= -fpic -I $(INCLUDESDIR) -I $(LIBFTDIR) $(COMMON)
LDFLAGS		?= -L $(LIBDIR) -lft $(COMMON)
SANITIZE	= -g3 -fsanitize=address

ifeq ($(VERBOSE),TRUE)
HIDE =  
else
HIDE = @
endif

$(NAME):	$(OBJS) $(LIBFT)
			@mkdir -p $(LIBDIR)
			@echo Linking $@
			$(HIDE)$(CC) $(LDFLAGS) -shared $^ -o $@

$(OBJSDIR)/%.o: $(SRCSDIR)/%.c
			@mkdir -p $(OBJSDIR)
			@echo Building $@
			$(HIDE)$(CC) -c $(CFLAGS) $< -o $@

all:		$(NAME)

$(LIBFT):
			$(HIDE)$(MAKE) -C $(LIBFTDIR)

$(BINDIR)/test_%: $(TESTDIR)/%.c
			@mkdir -p $(BINDIR)
			@echo Building $@
			$(HIDE)$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-rpath,$(LIBDIR) -l$(LIBRARY_NAME) $(TEST_COMMON) $< -o $@

test:		all $(TEST_BINS)
			$(HIDE)bash $(TESTDIR)/run_tests.sh $(BINDIR)

debug:		COMMON += $(SANITIZE)
debug:		re

clean:
			$(HIDE)$(RM) $(OBJSDIR)
			@echo cleaning done!

fclean:		clean
			$(HIDE)$(RM) $(BUILDDIR)/*
			@echo fcleaning done!

re:			fclean all

.PHONY:		all clean fclean re debug test
