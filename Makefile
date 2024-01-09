PROJDIR		:= $(realpath $CURDIR))
BUILDDIR	:= $(PROJDIR)/build
VERBOSE		:= FALSE

SRCS		:=
OBJS		:= $(SRCS:.c=.o)
NAME		:= libft_malloc_$(HOSTTYPE).so
CC			:= cc
RM			:= rm -f
COMMON		= -Wall -Werror -Wextra
CFLAGS		?= -I includes/ -I libft/ $(COMMON)
LDFLAGS		?= -L libft/ -lft $(COMMON)

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

ifeq ($(VERBOSE),TRUE)
HIDE =  
else
HIDE = @
endif

$(NAME):	$(OBJS) $(LIBFT)
			@echo Linking $@
			$(CC) $(LDFLAGS) $^ -o $@

all:		$(NAME)

clean:
			$(RM) $(OBJS)
			@echo cleaning done!

fclean:		clean
			$(RM) $(NAME)
			@echo fcleaning done!

re:			fclean all

.PHONY:		all clean fclean re

