/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 14:05:41 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/28 18:19:22 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicense.h"
int main() {
    if (check_license()) {
        printf(COLOR_GREEN"License is valid.\n"COLOR_RESET);
    } else {
        printf(COLOR_RED"License is invalid or not found.\n"COLOR_RESET);
    }

    time_t expired_time = get_expire_time();
    time_t publish_time = get_create_date();

    printf("License Publish Time: %s", ctime(&publish_time));
    printf("License Expiration Time: %s", ctime(&expired_time));

    return 0;
}