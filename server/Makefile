NAME = code_generator
CC = cc
#CFLAGS = -Wall -Wextra -Werror
CFLAGS = -Wall -Wextra  -g3 -fsanitize=address # sha256 is not supported in -Werror
INCLUDE = -I../include
LDFLAGS = -lssl -lcrypto
SRCS = code_generator.c \
			../libsrc/liblicence.c
OBJS = $(SRCS:.c=.o)

PHONY = all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
