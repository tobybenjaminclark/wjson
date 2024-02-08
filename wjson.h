/*
 * MiT License :: Copyright (C) 2023 Toby Benjamin Clark. All hail Gallaxhar!
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
#define WJSON_TYPE_LIST 5
#define WJSON_TYPE_NULL 6

/*
 * @brief Represents a node in a wJSON structure.
 *
 * This struct is used to create a tree-like structure to represent JSON data,
 * with support for wide characters. It includes fields for type information,
 * previous and next pointers for navigating the tree, a key for object members,
 * and a union for different data types (string, numerical, object, list, boolean).
 */
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
        struct wjson* data_list;
        bool data_bool;
    };
};

struct wjson* wjson_initialize_list();
int wjson_list_append_string(struct wjson* wjson_node, wchar_t* value);
int wjson_list_append_object(struct wjson* wjson_node, struct wjson* value);
int wjson_list_append_numerical(struct wjson* wjson_node, double value);
int wjson_list_append_list(struct wjson* wjson_node, struct wjson* value);
int wjson_list_append_boolean(struct wjson* wjson_node, bool value);

struct wjson* wjson_initialize();
struct wjson* wjson_parse_subobj(FILE* file);
int wjson_append_string(struct wjson* wjson_node, wchar_t* key, wchar_t* value);
int wjson_append_object(struct wjson* wjson_node, wchar_t* key, struct wjson* value);
int wjson_append_list(struct wjson* wjson_node, wchar_t* key, struct wjson* value);
int wjson_append_numerical(struct wjson* wjson_node, wchar_t* key, double value);
int wjson_append_boolean(struct wjson* wjson_node, wchar_t* key, bool value);

void wjson_print(struct wjson* head, int indentation);
void wjson_test();

#endif
