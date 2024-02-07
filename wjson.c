/*
 * GPL v3 - MPIR Compiler :: Copyright (C) 2023 Toby Benjamin Clark. All hail Gallaxhar!
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, of version 3 or later - See LICENSE for full terms of use.
 */

#include "wjson.h"

void wjson_print_list(struct wjson* head)
{
    struct wjson* current = head;
    printf("[\n");
    while (current != NULL)
    {
        printf("Type: %u\n", current->type);
        printf("Index: %ls\n", current->key);

        if (current->type == WJSON_TYPE_STRING)
        {
            printf("Data (String): %ls\n", current->data_string);
        }
        else if (current->type == WJSON_TYPE_NUMERICAL)
        {
            printf("Data (Numeric): %Lf\n", current->data_numerical);
        }
        else if (current->type == WJSON_TYPE_OBJECT)
        {
            printf("Data (Object): (Printing nested object)\n");
            wjson_print(current->data_object);
        }
        else if (current->type == WJSON_TYPE_BOOLEAN)
        {
            printf("Data (Boolean): %s\n", current->data_bool ? "true" : "false");
        }
        else if (current->type == WJSON_TYPE_LIST)
        {
            wjson_print_list(current->data_list);
        }

        printf(",\n");

        current = current->next;
    }
    printf("]\n");
}

void wjson_print(struct wjson* head) {
    struct wjson* current = head;

    while (current != NULL)
    {
        printf("Type: %u\n", current->type);
        printf("Key: %ls\n", current->key);

        if (current->type == WJSON_TYPE_STRING)
        {
            printf("Data (String): %ls\n", current->data_string);
        }
        else if (current->type == WJSON_TYPE_NUMERICAL)
        {
            printf("Data (Numeric): %Lf\n", current->data_numerical);
        }
        else if (current->type == WJSON_TYPE_OBJECT)
        {
            printf("Data (Object): (Printing nested object)\n");
            wjson_print(current->data_object);
        }
        else if (current->type == WJSON_TYPE_BOOLEAN)
        {
            printf("Data (Boolean): %s\n", current->data_bool ? "true" : "false");
        }
        else if (current->type == WJSON_TYPE_LIST)
        {
            wjson_print_list(current->data_list);
        }

        printf("\n");

        current = current->next;
    }
}





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
    /* Go to the end of the list */
    while (wjson_node->next != NULL) wjson_node = wjson_node->next;

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
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
        free(new_node->key);
        free(new_node->data_string);
        return 0;
    }

    return 1;
}

int wjson_append_object(struct wjson* wjson_node, wchar_t* key, struct wjson* value)
{
    /* Go to the end of the list */
    while (wjson_node->next != NULL) wjson_node = wjson_node->next;

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_OBJECT;
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_object to value */
    new_node->data_object = value;

    return 1;
}

int wjson_append_list(struct wjson* wjson_node, wchar_t* key, struct wjson* value)
{
    /* Go to the end of the list */
    while (wjson_node->next != NULL) wjson_node = wjson_node->next;

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_LIST;
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_object to value */
    new_node->data_object = value;

    return 1;
}

int wjson_append_numerical(struct wjson* wjson_node, wchar_t* key, double value)
{
    /* Go to the end of the list */
    while (wjson_node->next != NULL) wjson_node = wjson_node->next;

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_NUMERICAL;
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_object to value */
    new_node->data_numerical = value;

    return 1;
}

int wjson_append_boolean(struct wjson* wjson_node, wchar_t* key, bool value)
{
    /* Go to the end of the list */
    while (wjson_node->next != NULL) wjson_node = wjson_node->next;

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_BOOLEAN;
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_object to value */
    new_node->data_bool = value;

    return 1;
}





struct wjson* wjson_initialize_list()
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

int wjson_list_append_string(struct wjson* wjson_node, wchar_t* value)
{
    /* Go to the end of the list */
    int index = 0;
    while (wjson_node->next != NULL)
    {
        index++;
        wjson_node = wjson_node->next;
    }

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_STRING;

    /* Converting index to string */
    int bufferSize = snprintf(NULL, 0, L"%d", index);
    wchar_t *buffer = (wchar_t *)malloc((bufferSize + 1) * sizeof(wchar_t));
    swprintf(buffer, bufferSize + 1, L"%d", index);
    new_node->key = wcsdup(buffer);

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
        free(new_node->key);
        free(new_node->data_string);
        return 0;
    }

    return 1;
}

int wjson_list_append_object(struct wjson* wjson_node, struct wjson* value)
{
    /* Go to the end of the list */
    int index = 0;
    while (wjson_node->next != NULL)
    {
        index++;
        wjson_node = wjson_node->next;
    }

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_OBJECT;

    /* Converting index to string */
    int bufferSize = snprintf(NULL, 0, L"%d", index);
    wchar_t *buffer = (wchar_t *)malloc((bufferSize + 1) * sizeof(wchar_t));
    swprintf(buffer, bufferSize + 1, L"%d", index);
    new_node->key = wcsdup(buffer);

    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    new_node->data_object = value;

    return 1;
}

int wjson_list_append_numerical(struct wjson* wjson_node, double value)
{
    /* Go to the end of the list */
    int index = 0;
    while (wjson_node->next != NULL)
    {
        index++;
        wjson_node = wjson_node->next;
    }

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_NUMERICAL;

    /* Converting index to string */
    int bufferSize = snprintf(NULL, 0, L"%d", index);
    wchar_t *buffer = (wchar_t *)malloc((bufferSize + 1) * sizeof(wchar_t));
    swprintf(buffer, bufferSize + 1, L"%d", index);
    new_node->key = wcsdup(buffer);

    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    new_node->data_numerical = value;

    return 1;
}

int wjson_list_append_list(struct wjson* wjson_node, struct wjson* value)
{
    /* Go to the end of the list */
    int index = 0;
    while (wjson_node->next != NULL)
    {
        index++;
        wjson_node = wjson_node->next;
    }

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_LIST;

    /* Converting index to string */
    int bufferSize = snprintf(NULL, 0, L"%d", index);
    wchar_t *buffer = (wchar_t *)malloc((bufferSize + 1) * sizeof(wchar_t));
    swprintf(buffer, bufferSize + 1, L"%d", index);
    new_node->key = wcsdup(buffer);

    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    new_node->data_list = value;

    return 1;
}

int wjson_list_append_boolean(struct wjson* wjson_node, bool value)
{
    /* Go to the end of the list */
    int index = 0;
    while (wjson_node->next != NULL)
    {
        index++;
        wjson_node = wjson_node->next;
    }

    /* Node is empty (first element in the list) */
    struct wjson* new_node;
    if (wjson_node->type == WJSON_TYPE_EMPTY)
        new_node = wjson_node;
    else
    {
        new_node = wjson_initialize();
        wjson_node->next = new_node;
        new_node->prev = wjson_node;
    }

    new_node->type = WJSON_TYPE_BOOLEAN;

    /* Converting index to string */
    int bufferSize = snprintf(NULL, 0, L"%d", index);
    wchar_t *buffer = (wchar_t *)malloc((bufferSize + 1) * sizeof(wchar_t));
    swprintf(buffer, bufferSize + 1, L"%d", index);
    new_node->key = wcsdup(buffer);

    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    new_node->data_bool = value;

    return 1;
}





void wjson_test()
{
    struct wjson* test = wjson_initialize();
    wjson_append_string(test, L"Key1", L"Val1");
    wjson_append_numerical(test, L"Key2", 32);
    wjson_append_string(test, L"Key3", L"Val3");

    struct wjson* test2 = wjson_initialize_list();
    wjson_list_append_string(test2, L"Val1");
    wjson_list_append_numerical(test2, 32);
    wjson_list_append_string(test2, L"Val3");

    wjson_append_list(test, "Key4", test2);

    wjson_print(test);
}