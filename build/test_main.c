/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 14:05:41 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/25 14:09:57 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicence.h"
// test
void some_main_function(int argc, char **argv, char **envp) {
    printf("hello world");
    (void)envp;
    (void)argv;
    (void)argc;
}

int main() {
    run_main_logic(some_main_function, 0, NULL, NULL, 3600);
    return 0;
}