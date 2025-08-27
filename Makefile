# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/25 14:05:28 by HaJuYoung(juha)   #+#    #+#              #
#    Updated: 2025/08/25 14:17:27 by HaJuYoung(juha)  ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.PHONY: all clean fclean re 

all: compile

compile:
	make -C libsrc
	make -C tools
	make -C server
	make -C build

fclean:
	make -C libsrc fclean
	make -C tools fclean
	make -C server fclean
	make -C build fclean
	rm -rf bin/main_test

re: fclean compile
