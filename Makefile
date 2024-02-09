PROJDIR		:= $(realpath $(CURDIR))
SRCSDIR		:= $(PROJDIR)/src
TESTDIR		:= $(PROJDIR)/test
INCLUDESDIR	:= $(PROJDIR)/includes
BUILDDIR	:= $(PROJDIR)/build
OBJSDIR		:= $(BUILDDIR)/objs
LIBDIR		:= $(BUILDDIR)/lib
BINDIR		:= $(BUILDDIR)/bin
VERBOSE		:= FALSE

SRCS_LIST	:= malloc.c
OBJS_LIST	:= $(SRCS_LIST:.c=.o)
SRCS		:= $(addprefix $(SRCSDIR)/,$(SRCS_LIST))
OBJS		:= $(addprefix $(OBJSDIR)/,$(OBJS_LIST))
TEST_SRCS	:= $(TESTDIR)/test.c
#LIBFT		:= libft/libft.a

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif
LIBRARY_NAME := ft_malloc_$(HOSTTYPE)

NAME		:= $(LIBDIR)/lib$(LIBRARY_NAME).so

CC			:= gcc
RM			:= rm -rf
COMMON		= -Wall -Werror -Wextra
CFLAGS		?= -fpic -I $(INCLUDESDIR) $(COMMON)
#CFLAGS		?= -fpic -I includes/ -I libft/ $(COMMON)
LDFLAGS		?= $(COMMON)
#LDFLAGS		?= -L libft/ -lft $(COMMON)
SANITIZE	= -g3 -fsanitize=address

ifeq ($(VERBOSE),TRUE)
HIDE =  
else
HIDE = @
endif

#$(NAME):	$(OBJS) $(LIBFT)
$(NAME):	$(OBJS)
			@mkdir -p $(LIBDIR)
			@echo Linking $@
			$(HIDE)$(CC) $(LDFLAGS) -shared $^ -o $@

$(OBJSDIR)/%.o: $(SRCSDIR)/%.c
			@mkdir -p $(OBJSDIR)
			@echo Building $@
			$(HIDE)$(CC) -c $(CFLAGS) $< -o $@

all:		$(NAME)

#$(LIBFT):
#			$(MAKE) -C libft/

test:		all
			@mkdir -p $(BINDIR)
			@echo Building $@
			$(HIDE)$(CC) $(CFLAGS) $(TEST_SRCS) $(LDFLAGS) -L $(LIBDIR) -Wl,-rpath,$(LIBDIR) -l$(LIBRARY_NAME) -o $(BINDIR)/$@

debug:		COMMON += $(SANITIZE)
debug:		re

clean:
			$(HIDE)$(RM) $(OBJSDIR)
#			$(HIDE)$(MAKE) clean -C libft/
			@echo cleaning done!

fclean:		clean
			$(HIDE)$(RM) $(BUILDDIR)/*
#			$(HIDE)$(MAKE) fclean -C libft/
			@echo fcleaning done!

re:			fclean all

.PHONY:		all clean fclean re debug
