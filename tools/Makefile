NAME = licence_generator
CC = cc
CFLAGS = -Wall -Wextra -Werror
INCLUDE = -I../include
LDFLAGS = -lssl -lcrypto
SRCS = generator.c \
	../libsrc/liblicence.c
OBJS = $(SRCS:.c=.o)

PHONY = all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
