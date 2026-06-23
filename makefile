NAME    = codexion
CC      = cc
CFLAGS  = -Wall -Wextra -Werror -pthread

SRCS    = main.c
OBJS    = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
run: $(NAME)
	./$(NAME) 3 800 200 200 200 3 100 fifo

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re