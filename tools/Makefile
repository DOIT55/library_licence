NAME = license_generator
CC = cc
CFLAGS = -Wall -Wextra -Werror #-g3 -fsanitize=address # sha256 is not supported in -Werror
INCLUDE = -I../include
LDFLAGS = -llicense -lssl -lcrypto 
LIBRARY = -L../lib 
SRCS = license_generator.c
OBJS = $(SRCS:.c=.o)

PHONY = all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LIBRARY) -o $@ $^ $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBRARY) -c $< -o $@ 

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
