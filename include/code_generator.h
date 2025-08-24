/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   code_generator.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung (juha) <contemplation.person@gma +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 22:39:30 by HaJuYoung (juha)  #+#    #+#             */
/*   Updated: 2025/08/24 17:21:08 by HaJuYoung (juha) ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "liblicence.h"
#define CODE_PASSWORD "he11oWor1dLicence!"

typedef struct {
    char request_user_name[MAX_USER_NAME_LENGTH];
    char equipment_name[MAX_EQUIPMENT_NAME_LENGTH];
    Crypt_info crypt_info;
    unsigned char licence_code[512];
} Code_generator_info;

#endif
