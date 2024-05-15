PROJDIR		:= $(realpath $(CURDIR)/../..)
BUILDDIR	:= $(PROJDIR)/build
LIBDIR		:= $(BUILDDIR)/lib
OBJSDIR		:= $(BUILDDIR)/objs
SRCS		= ft_strlen.c \
			  ft_memset.c \
			  ft_memcpy.c \
			  ft_memccpy.c \
			  ft_memmove.c \
			  ft_memchr.c \
			  ft_memcmp.c \
			  ft_putchar_fd.c \
			  ft_putstr_fd.c \
			  ft_putendl_fd.c \
			  ft_putnbr_fd.c \
			  ft_lstnew.c \
			  ft_lstadd_front.c \
			  ft_lstsize.c \
			  ft_lstlast.c \
			  ft_lstadd_back.c \
			  ft_lstdelone.c \
			  ft_lstclear.c \
			  ft_lstiter.c \
			  ft_lstmap.c
OBJS_LIST	:= $(SRCS:.c=.o)
OBJS		:= $(addprefix $(OBJSDIR)/,$(OBJS_LIST))
LIBRARY_NAME := libft.a
NAME		= $(LIBDIR)/$(LIBRARY_NAME)
CC			= gcc
RM			= rm -rf
CFLAGS		= -Wall -Werror -Wextra

ifeq ($(VERBOSE),TRUE)
HIDE =  
else
HIDE = @
endif

$(NAME):	$(OBJS)
			@mkdir -p $(LIBDIR)
			@echo Linking $@
			$(HIDE)ar rc $(NAME) $(OBJS)
			$(HIDE)ranlib $(NAME)

$(OBJSDIR)/%.o: %.c
			@mkdir -p $(OBJSDIR)
			@echo Building $@
			$(HIDE)$(CC) -c $(CFLAGS) $< -o $@

all:		$(NAME)

clean:
			$(HIDE)$(RM) $(OBJS)

fclean:		clean
			$(HIDE)$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
