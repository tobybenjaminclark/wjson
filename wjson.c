/*
 * GPL v3 - MPIR Compiler :: Copyright (C) 2023 Toby Benjamin Clark. All hail Gallaxhar!
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, of version 3 or later - See LICENSE for full terms of use.
 */

#include "wjson.h"

void print_wjson_list(struct wjson* head) {
    struct wjson* current = head;

    while (current != NULL) {
        printf("Type: %u\n", current->type);
        printf("Key: %ls\n", current->key);

        if (current->type == WJSON_TYPE_STRING) {
            printf("Data (String): %ls\n", current->data_string);
        } else if (current->type == WJSON_TYPE_NUMERICAL) {
            printf("Data (Numeric): %Lf\n", current->data_numerical);
        } else if (current->type == WJSON_TYPE_OBJECT) {
            printf("Data (Object): (Printing nested object)\n");
            print_wjson_list(current->data_object);
        } else if (current->type == WJSON_TYPE_BOOLEAN) {
            printf("Data (Boolean): %s\n", current->data_bool ? "true" : "false");
        }

        printf("\n");

        current = current->next;
    }
}

// Function to create a new wjson node and initialize its members
struct wjson* wjson_initialize()
{
    /* Allocating Memory for new struct wjson pointer */
    struct wjson* new_node = (struct wjson*)malloc(sizeof(struct wjson));
    if (new_node == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson instance.");
        exit(EXIT_FAILURE);
    }

    /* Initialize Members to NULL */
    new_node->type = 0;
    new_node->prev = NULL;
    new_node->next = NULL;
    new_node->key = NULL;

    /* Initialize Union to NULL */
    new_node->data_string = NULL;
    new_node->data_numerical = 0.0;
    new_node->data_object = NULL;
    new_node->data_bool = false;

    return new_node;
}

int wjson_append_string(struct wjson* wjson_node, wchar_t* key, wchar_t* value)
{
    /* Node is empty (first element in list) */
    struct wjson* new_node;
    if(wjson_node->type == WJSON_TYPE_EMPTY) new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_STRING;
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    new_node->data_string = wcsdup(value);
    if (new_node->data_string == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson String.");
        free(new_node->data_string);
        free(new_node->key);
        return 0;
    }

    return 1;
}

void wjson_test()
{
    struct wjson* test = wjson_initialize();
    wjson_append_string(test, L"Key1", L"Val1");
    wjson_append_string(test, L"Key2", L"Val2");
    print_wjson_list(test);
}