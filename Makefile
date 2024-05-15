PROJDIR		:= $(realpath $(CURDIR))
SRCSDIR		:= $(PROJDIR)/src
TESTDIR		:= $(PROJDIR)/test
INCLUDESDIR	:= $(PROJDIR)/includes
BUILDDIR	:= $(PROJDIR)/build
OBJSDIR		:= $(BUILDDIR)/objs
LIBDIR		:= $(BUILDDIR)/lib
BINDIR		:= $(BUILDDIR)/bin
VERBOSE		:= FALSE

SRCS_LIST	:= malloc.c heap.c
OBJS_LIST	:= $(SRCS_LIST:.c=.o)
SRCS		:= $(addprefix $(SRCSDIR)/,$(SRCS_LIST))
OBJS		:= $(addprefix $(OBJSDIR)/,$(OBJS_LIST))
TEST_SRCS	:= $(TESTDIR)/test.c
LIBFTDIR	:= ${SRCSDIR}/libft
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

test:		all
			@mkdir -p $(BINDIR)
			@echo Building $@
			$(HIDE)$(CC) $(CFLAGS) $(TEST_SRCS) $(LDFLAGS) -Wl,-rpath,$(LIBDIR) -l$(LIBRARY_NAME) -o $(BINDIR)/$@

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
