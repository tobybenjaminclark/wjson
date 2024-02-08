/*
 * GPL v3 - MPIR Compiler :: Copyright (C) 2023 Toby Benjamin Clark. All hail Gallaxhar!
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, of version 3 or later - See LICENSE for full terms of use.
 */

#include "wjson.h"

int matchSequence(FILE* file, const wchar_t* sequence)
{
    int index = 0;
    wint_t wchar;
    wint_t buffer[1024]; // Assuming a maximum sequence length of 1023 characters
    int bufferIndex = 0;

    // Save the current file position
    long startPos = ftell(file);

    while ((wchar = fgetwc(file)) != WEOF && sequence[index] != L'\0') {
        if (wchar != sequence[index]) {
            // Characters do not match, put them back into the stream
            while (bufferIndex > 0) {
                ungetwc(buffer[--bufferIndex], file);
            }
            // Restore the file position to the initial position
            fseek(file, startPos, SEEK_SET);
            return 0; // Return failure
        }
        buffer[bufferIndex++] = wchar;
        index++;
    }

    if (sequence[index] == L'\0') {
        return 1; // Return success if end of sequence is reached
    } else {
        // Restore the file position to the initial position
        fseek(file, startPos, SEEK_SET);
        return 0; // Return failure if end of file is reached before the full sequence
    }
}

wchar_t* wjson_parse_value_string(FILE* file, wint_t wchar)
{
    wint_t buffer[1024];
    int bufferIndex = 0;

    while ((wchar = fgetwc(file)) != WEOF && wchar != L'"') buffer[bufferIndex++] = wchar;

    /* Null Terminate String */
    buffer[bufferIndex] = L'\0';
    wprintf(L"v\"%ls\"\n", buffer);
    return buffer;
}

double parseDouble(FILE* file, wint_t firstChar)
{
    wchar_t buffer[1024]; // Assuming a maximum length for the numerical value
    int bufferIndex = 0;

    buffer[bufferIndex++] = firstChar;

    wint_t wchar;
    while ((wchar = fgetwc(file)) != WEOF && (iswdigit(wchar) || wchar == L'.' || wchar == L'-')) {
        buffer[bufferIndex++] = wchar;
    }

    buffer[bufferIndex] = L'\0'; // Null-terminate the buffer to form a string

    double result;
    swscanf(buffer, L"%lf", &result);
    return result;
}

/*
 *  int bufferSize = snprintf(NULL, 0, L"%d", index);
    wchar_t *buffer = (wchar_t *)malloc((bufferSize + 1) * sizeof(wchar_t));
    swprintf(buffer, bufferSize + 1, L"%d", index);
    new_node->key = wcsdup(buffer);
 */
struct wjson* wjson_parse_list(FILE* file)
{

    wint_t wchar;
    if ((wchar = fgetwc(file)) != L'[') return NULL;

    struct wjson* list_node = wjson_initialize_list();

    while((wchar = fgetwc(file)) != L']' && wchar != WEOF)
    {
        if (wchar == L'"')
        {
            wchar_t* parsed_string = wjson_parse_value_string(file, wchar);
            wjson_list_append_string(list_node, parsed_string);
        }
        else if (wchar == L'{')
        {
            /* Parse subobject */
            printf("subobj");
            ungetwc(wchar, file);
            wjson_list_append_object(list_node, wjson_parse_subobj(file));
        }
        else if (wchar == L'[')
        {
            /* Parse subobject */
            printf("List");
            ungetwc(wchar, file);
            wjson_list_append_object(list_node, wjson_parse_list(file));
        }
        else if (matchSequence(file, L"true"))
        {
            printf("True\n");
            wjson_list_append_boolean(list_node, true);
        }
        else if (matchSequence(file, L"false"))
        {
            printf("False\n");
            wjson_list_append_boolean(list_node, false);
        }
        else if (matchSequence(file, L"null"))
        {
            printf("Null\n");
        }
        else if (iswdigit(wchar) || wchar == L'.' || wchar == L'-')
        {
            double parsedValue = parseDouble(file, wchar);
            wprintf(L"%f\n", parsedValue);
            wjson_list_append_numerical(list_node, parsedValue);
        }
        else
        {
            continue;
        }
    }
    return list_node;
}

void wjson_parse_value(FILE* file, wint_t key[1024], struct wjson* wjson_node)
{
    wint_t wchar;
    while ((wchar = fgetwc(file)) != WEOF)
    {
        if(wchar == L'"')
        {
            wchar_t* parsed_string = wjson_parse_value_string(file, wchar);
            wjson_append_string(wjson_node, key, parsed_string);
            return;
        }
        else if(wchar == L'{')
        {
            printf("subobj");
            ungetwc(wchar, file);
            wjson_append_object(wjson_node, key, wjson_parse_subobj(file));
            return;
        }
        else if(wchar == L'[')
        {
            /* Parse subobject */
            printf("List");
            ungetwc(wchar, file);
            wjson_append_list(wjson_node, key, wjson_parse_list(file));
            return;
        }
        else if(matchSequence(file, L"true"))
        {
            printf("True\n");
            wjson_append_boolean(wjson_node, key, true);
            return;
        }
        else if(matchSequence(file, L"false"))
        {
            printf("False\n");
            wjson_append_boolean(wjson_node, key, false);
            return;
        }
        else if(matchSequence(file, L"null"))
        {
            wjson_append_string(wjson_node, key, L"null");
            return;
        }
        else if(iswdigit(wchar) || wchar == L'.' || wchar == L'-')
        {
            double parsedValue = parseDouble(file, wchar);
            wjson_append_numerical(wjson_node, key, parsedValue);
            return;
        }
        continue;
    }
}

void wjson_parse_key(FILE* file, wint_t wchar, struct wjson* wjson_node)
{
    if (wchar != L'"') return;

    /* Accumulate characters into a buffer until closing double quote is encountered */
    wint_t buffer[1024];
    int bufferIndex = 0;

    while ((wchar = fgetwc(file)) != WEOF && wchar != L'"') buffer[bufferIndex++] = wchar;

    /* Null Terminate String */
    buffer[bufferIndex] = L'\0';
    wprintf(L"k\"%ls\"   ", buffer);

    /* Parse : */
    while ((wchar = fgetwc(file)) != WEOF && wchar != L':') continue;

    /* Parse Value */
    wjson_parse_value(file, buffer, wjson_node);
}

struct wjson* wjson_parse_subobj(FILE* file)
{
    struct wjson* wjson_node = wjson_initialize();

    printf("\nSUBOBJ\n");
    wint_t wchar;
    while ((wchar = fgetwc(file)) != WEOF && wchar != L'{') continue;
    while ((wchar = fgetwc(file)) != WEOF && wchar != L'}')
    {
        wjson_parse_key(file, wchar, wjson_node);
    }
    printf("END SUBOBJ\n");
    return wjson_node;
}

struct wjson* wjson_parse(const char* filename)
{
    FILE* file = fopen(filename, "r");

    if (file == NULL)
    {
        wprintf(L"Error opening file: %s\n", filename);
        return NULL;
    }
    struct wjson* wjson_node = wjson_parse_subobj(file);


    fclose(file);
    return wjson_node;
}




void wjson_print_indentation(int indentation)
{
    int indentation_counter = 0;
    for(indentation_counter = 0; indentation_counter < indentation; indentation_counter++)
        wprintf(L"\t");
    return;
}

void wjson_print_list(struct wjson* head, int indentation)
{
    struct wjson* current = head;
    wprintf(L"[\n");
    while (current != NULL)
    {
        wjson_print_indentation(indentation + 1);
        switch (current->type)
        {
            case WJSON_TYPE_NUMERICAL:
                wprintf(L"%f", current->data_numerical);
                break;
            case WJSON_TYPE_STRING:
                wprintf(L"\"%ls\"", current->data_string);
                break;
            case WJSON_TYPE_BOOLEAN:
                if(current->data_bool) wprintf(L"true");
                else wprintf(L"false");
                break;
            case WJSON_TYPE_OBJECT:
                wjson_print(current->data_object, indentation + 1);
                break;
            case WJSON_TYPE_LIST:
                wjson_print_list(current->data_list, indentation + 1);
                break;
            case WJSON_TYPE_NULL:
                wprintf(L"null");
                break;
        }

        /* If not the last entry */
        if(current->next != NULL) wprintf(L",");
        printf("\n");

        current = current->next;
    }
    wjson_print_indentation(indentation);
    wprintf(L"]");
}

void wjson_print(struct wjson* head, int indentation)
{
    struct wjson* current = head;
    wprintf(L"{\n");
    while (current != NULL)
    {
        wjson_print_indentation(indentation + 1);
        printf("\"%ls\" : ", current->key);
        switch (current->type)
        {
            case WJSON_TYPE_NUMERICAL:
                wprintf(L"%f", current->data_numerical);
                break;
            case WJSON_TYPE_STRING:
                wprintf(L"\"%ls\"", current->data_string);
                break;
            case WJSON_TYPE_BOOLEAN:
                if(current->data_bool) wprintf(L"true");
                else wprintf(L"false");
                break;
            case WJSON_TYPE_OBJECT:
                wjson_print(current->data_object, indentation + 1);
                break;
            case WJSON_TYPE_LIST:
                wjson_print_list(current->data_list, indentation + 1);
                break;
            case WJSON_TYPE_NULL:
                wprintf(L"null");
                break;
        }

        /* If not the last entry */
        if(current->next != NULL) wprintf(L",");
        printf("\n");

        current = current->next;
    }
    wjson_print_indentation(indentation);
    wprintf(L"}");
}

∂


/*
 * @brief Initializes a new wjson instance and allocates memory for it.
 *
 * This function allocates memory for a new wjson instance, initializes its
 * members to default values (NULL, 0, false), and returns a pointer to the
 * newly created instance.
 *
 * @return Pointer to the newly initialized wjson instance.
 * @note Memory allocation failure results in an error message and program exit.
 * @author Toby Benjamin Clark
 */
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

/* @
 * @brief Appends a new string element to the wjson structure.
 *
 * This function appends a new wjson node with string data to the end of the
 * provided wjson_node list. It allocates memory for the key and value, sets
 * the necessary fields, and updates the list pointers accordingly.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param key The key for the new element.
 * @param value The string value for the new element.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Allocate memory for the key and copy its value */
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Allocate memory for the string value and copy its value */
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

/* @function wjson_append_object
 * @brief Appends a new object element to the wjson structure.
 *
 * This function appends a new wjson node with object data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_object field is
 * updated to point to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param key The key for the new element.
 * @param value The wjson node representing the object value.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Allocate memory for the key and copy its value */
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_object to point to the provided value */
    new_node->data_object = value;

    return 1;
}

/* @function wjson_append_list
 * @brief Appends a new list element to the wjson structure.
 *
 * This function appends a new wjson node with list data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_object field is
 * updated to point to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param key The key for the new element.
 * @param value The wjson node representing the list value.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Allocate memory for the key and copy its value */
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_object to point to the provided value */
    new_node->data_object = value;

    return 1;
}

/* @function wjson_append_numerical
 * @brief Appends a new numerical element to the wjson structure.
 *
 * This function appends a new wjson node with numerical data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_numerical field
 * is updated to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param key The key for the new element.
 * @param value The numerical value for the new element.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Allocate memory for the key and copy its value */
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_numerical to the provided value */
    new_node->data_numerical = value;

    return 1;
}

/* @function wjson_append_boolean
 * @brief Appends a new boolean element to the wjson structure.
 *
 * This function appends a new wjson node with boolean data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_bool field is
 * updated to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param key The key for the new element.
 * @param value The boolean value for the new element.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Allocate memory for the key and copy its value */
    new_node->key = wcsdup(key);
    if (new_node->key == NULL)
    {
        fprintf(stderr, "wJson: Failed to allocate memory for new wJson Key.");
        free(new_node->key);
        return 0;
    }

    /* Update data_bool to the provided value */
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
    struct wjson* test_parse = wjson_parse("test.json");
    wjson_print(test_parse, 0);

    struct wjson* test = wjson_initialize();
    wjson_append_string(test, L"Key1", L"Val1");
    wjson_append_numerical(test, L"Key2", 32);
    wjson_append_boolean(test, L"Key3", true);

    struct wjson* nested = wjson_initialize();
    wjson_append_string(nested, L"Nested1", L"NestedVal1");
    wjson_append_string(nested, L"Nested2", L"NestedVal2");

    struct wjson* list = wjson_initialize_list();
    wjson_list_append_string(list, L"ListVal1");
    wjson_list_append_string(list, L"ListVal2");

    wjson_append_list(test, L"ListKey",list);
    wjson_append_object(test, L"Nest!", nested);
    wjson_print(test, 0);
}