NAME    = codexion
CC      = cc
CFLAGS  = -Wall -Wextra -Werror -pthread -g

SRCS    = main.c \
	      parsing.c \
	      errors.c \
	      setup.c \
	      dongle.c \
	      release.c \
	      dongle_queue.c \
	      coder.c \
	      monitor.c
OBJS    = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	./$(NAME) 3 800 200 200 200 3 100 edf

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re