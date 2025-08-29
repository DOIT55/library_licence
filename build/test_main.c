/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 14:05:41 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/29 14:49:39 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicense.h"

int main() {
    int code = 0;

    code = load_license_file();
    if (code != Result_success) {
        printf("License Load Error: %s\n %d\n", license_error_code_to_string(code),code);
        return 0;
    }

    time_t expired_time = get_expire_time();
    time_t publish_time = get_create_date();

    printf("License Publish Time: %s", ctime(&publish_time));
    printf("License Expiration Time: %s", ctime(&expired_time));

    if (!is_license_valid_period()) {
        printf("invalid license\n");
        return 0;
    }


    return 0;
}