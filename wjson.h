/*
 * GPL v3 - MPIR Compiler :: Copyright (C) 2023 Toby Benjamin Clark. All hail Gallaxhar!
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, of version 3 or later - See LICENSE for full terms of use.
 */

#ifndef WJSON_WJSON_H
#define WJSON_WJSON_H

#include <wchar.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define WJSON_TYPE_EMPTY 0
#define WJSON_TYPE_STRING 1
#define WJSON_TYPE_NUMERICAL 2
#define WJSON_TYPE_OBJECT 3
#define WJSON_TYPE_BOOLEAN 4
#define WJSON_TYPE_NULL 5


struct wjson
{
    unsigned short int type;

    struct wjson* prev;
    struct wjson* next;

    wchar_t* key;
    union {
        wchar_t* data_string;
        long double data_numerical;
        struct wjson* data_object;
        bool data_bool;
    };
};

void wjson_test();

#endif
