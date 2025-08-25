AR = ar
AR_FLAGS = rcs
NAME = test_main
LIB_NAME = liblicence.a
CC = cc
CFLAGS = -Wall -Wextra -Werror #-g3 -fsanitize=address
INCLUDE = -I../include
LDFLAGS = -llicence -lssl -lcrypto 

LIB_SRCS = liblicence.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

TEST_SRCS = test_main.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY: all clean fclean re lib test

all: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJS)
	$(AR) $(AR_FLAGS) $@ $^
	mkdir -p ../lib
	cp -f $@ ../lib
	@echo "Static library $(LIB_NAME) created successfully"

lib: $(LIB_NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(LIB_NAME)

re: fclean all
