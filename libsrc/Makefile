AR = ar
AR_FLAGS = rcs
NAME = test_main
LIB_NAME = liblicence.a
CC = cc
CFLAGS = -Wall -Wextra  -g3 -fsanitize=address
INCLUDE = -I../include
LDFLAGS = -lssl -lcrypto -llicence

LIB_SRCS = liblicence.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# 테스트 프로그램 소스 파일들
TEST_SRCS = test_main.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY: all clean fclean re lib test

all: $(LIB_NAME)

# 정적 라이브러리 생성
$(LIB_NAME): $(LIB_OBJS)
	$(AR) $(AR_FLAGS) $@ $^
	mkdir -p ../lib
	cp $@ ../lib
	@echo "Static library $(LIB_NAME) created successfully"

# 테스트 프로그램 생성 (정적 라이브러리 사용)
$(NAME): $(TEST_OBJS) $(LIB_NAME)
	$(CC) $(CFLAGS) $(LIBRARY) -o $@ $(TEST_OBJS) -L. $(LDFLAGS)

lib: $(LIB_NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(NAME) $(LIB_NAME)

re: fclean all

test: $(NAME)
	./$(NAME)
