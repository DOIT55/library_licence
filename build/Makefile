NAME = main_test
CC = cc
CFLAGS = -Wall -Wextra -Werror #-g3 -fsanitize=address # sha256 is not supported in -Werror
INCLUDE = -I../include
LDFLAGS = -llicense -lssl -lcrypto 
LIBRARY = -L../lib 
SRCS = test_main.c
OBJS = $(SRCS:.c=.o)

PHONY = all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	mkdir -p ../bin
	$(CC) $(CFLAGS) $(LIBRARY) -o $@ $^ $(LDFLAGS) 
	mv -f $@ ../bin/

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBRARY) -c $< -o $@ 

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
