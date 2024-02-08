/*
 * GPL v3 - MPIR Compiler :: Copyright (C) 2023 Toby Benjamin Clark. All hail Gallaxhar!
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, of version 3 or later - See LICENSE for full terms of use.
 */

#include "wjson.h"

/**
 * @brief Matches a sequence of wide characters in the input JSON file stream.
 *
 * This function attempts to match the given sequence of wide characters in the input file stream.
 * If the sequence is found, the file stream position is unchanged, and the function returns 1.
 * If the sequence is not found, the file stream is rewound to its initial position, and the function returns 0.
 *
 * @param file FILE pointer to the input JSON file.
 * @param sequence Wide character sequence to match in the input file stream.
 * @return 1 if the sequence is successfully matched, 0 otherwise.
 */
int wjson_match_sequence(FILE* file, const wchar_t* sequence)
{
    int index = 0;
    wint_t wchar;
    wint_t buffer[1024];
    int bufferIndex = 0;

    /* Save the current file position to startPos */
    long startPos = ftell(file);

    while ((wchar = fgetwc(file)) != WEOF && sequence[index] != L'\0')
    {
        if (wchar != sequence[index])
        {

            /* Characters do not match, put them back into the stream */
            while (bufferIndex > 0)
            {
                ungetwc(buffer[--bufferIndex], file);
            }

            /* Restore the file position to the initial position */
            fseek(file, startPos, SEEK_SET);
            return 0;
        }
        buffer[bufferIndex++] = wchar;
        index++;
    }

    if (sequence[index] == L'\0')
    {
        /* Return success if end of sequence is reached */
        return 1;
    }
    else
    {
        /* Restore the file position to the initial position & return failure. */
        fseek(file, startPos, SEEK_SET);
        return 0;
    }
}


/**
 * @brief Parses a JSON string value from the file stream and returns a wchar_t pointer.
 *
 * This function reads characters from the file stream until a closing double quote is encountered,
 * storing them in a buffer and null-terminating the string. The buffer is then returned as a wchar_t pointer.
 *
 * @param file FILE pointer to the input JSON file.
 * @param wchar Initial wide character, expected to be the opening double quote of the string.
 * @return wchar_t pointer to the parsed string.
 * @note Memory for the string is allocated on the stack and must be used with caution.
 * @author Toby Benjamin Clark
 */
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

/**
 * @brief Parses a JSON numeric value from the file stream and returns a double.
 *
 * This function reads characters from the file stream until a non-numeric character is encountered,
 * storing them in a buffer. The buffer is then null-terminated and parsed as a double value.
 *
 * @param file FILE pointer to the input JSON file.
 * @param firstChar Initial wide character, expected to be the first character of the numeric value.
 * @return Parsed double value.
 * @note The buffer is null-terminated to form a string before parsing.
 * @author Toby Benjamin Clark
 */
double wjson_parse_double(FILE* file, wint_t firstChar)
{
    wchar_t buffer[1024];
    int bufferIndex = 0;

    buffer[bufferIndex++] = firstChar;

    /* Append characters while they are numerical */
    wint_t wchar;
    while ((wchar = fgetwc(file)) != WEOF && (iswdigit(wchar) || wchar == L'.' || wchar == L'-'))
    {
        buffer[bufferIndex++] = wchar;
    }

    /* Null Terminate the Buffer */
    buffer[bufferIndex] = L'\0';

    /* Calculate & return result */
    double result;
    swscanf(buffer, L"%lf", &result);
    return result;
}

/**
 * @brief Parses a JSON array from the file stream and returns a wjson pointer.
 *
 * This function reads characters from the file stream, parsing values until the end of the array is reached.
 * Parsed values are appended to a wjson list. The resulting wjson list is then returned.
 *
 * @param file FILE pointer to the input JSON file.
 * @return wjson pointer to the parsed JSON array.
 * @note The caller is responsible for freeing the allocated memory.
 * @author Toby Benjamin Clark
 */
struct wjson* wjson_parse_list(FILE* file)
{
    wint_t wchar;
    /* If the first character is not an opening square bracket, return NULL */
    if ((wchar = fgetwc(file)) != L'[') return NULL;

    /* Initialize a wjson list to store parsed values */
    struct wjson* list_node = wjson_initialize_list();

    /* Loop through characters until the end of the array or end of file is reached */
    while ((wchar = fgetwc(file)) != L']' && wchar != WEOF)
    {
        /* If the character is a double quote, parse a string value and append it to the list */
        if (wchar == L'"')
        {
            wchar_t* parsed_string = wjson_parse_value_string(file, wchar);
            wjson_list_append_string(list_node, parsed_string);
        }
            /* If the character is an opening curly brace, parse an object value and append it to the list */
        else if (wchar == L'{')
        {
            ungetwc(wchar, file);
            wjson_list_append_object(list_node, wjson_parse_subobj(file));
        }
            /* If the character is an opening square bracket, parse a nested array value and append it to the list */
        else if (wchar == L'[')
        {
            ungetwc(wchar, file);
            wjson_list_append_object(list_node, wjson_parse_list(file));
        }
            /* If the character sequence matches "true", parse a boolean true value and append it to the list */
        else if (wjson_match_sequence(file, L"true"))
        {
            wjson_list_append_boolean(list_node, true);
        }
            /* If the character sequence matches "false", parse a boolean false value and append it to the list */
        else if (wjson_match_sequence(file, L"false"))
        {
            wjson_list_append_boolean(list_node, false);
        }
            /* If the character sequence matches "null", parse a null value and append it to the list */
        else if (wjson_match_sequence(file, L"null"))
        {
            wjson_list_append_string(list_node, L"null");
        }
            /* If the character is a digit, dot, or minus sign, parse a numerical value and append it to the list */
        else if (iswdigit(wchar) || wchar == L'.' || wchar == L'-')
        {
            double parsedValue = wjson_parse_double(file, wchar);
            wjson_list_append_numerical(list_node, parsedValue);
        }
            /* Continue to the next character if none of the conditions are met */
        else
        {
            continue;
        }
    }
    return list_node;
}

/**
 * @brief Parses a JSON value from the file stream and appends it to a wjson node.
 *
 * This function reads characters from the file stream, determining the type of JSON value,
 * and appends it to the provided wjson node using the specified key.
 *
 * @param file FILE pointer to the input JSON file.
 * @param key Array of wide characters representing the key associated with the JSON value.
 * @param wjson_node Pointer to the wjson node to which the parsed value will be appended.
 * @note The key is used for identifying the field within the wjson node.
 * @note The caller is responsible for freeing the allocated memory for the key.
 * @note If a string value is parsed, its memory is allocated on the stack and must be used with caution.
 * @author Toby Benjamin Clark
 */
void wjson_parse_value(FILE* file, wint_t key[1024], struct wjson* wjson_node)
{
    wint_t wchar;
    while ((wchar = fgetwc(file)) != WEOF)
    {
        /* If the character is a double quote, parse a string value */
        if (wchar == L'"')
        {
            wchar_t* parsed_string = wjson_parse_value_string(file, wchar);
            wjson_append_string(wjson_node, key, parsed_string);
            return;
        }
            /* If the character is an opening curly brace, parse an object value */
        else if (wchar == L'{')
        {
            ungetwc(wchar, file);
            wjson_append_object(wjson_node, key, wjson_parse_subobj(file));
            return;
        }
            /* If the character is an opening square bracket, parse an array value */
        else if (wchar == L'[')
        {
            ungetwc(wchar, file);
            wjson_append_list(wjson_node, key, wjson_parse_list(file));
            return;
        }
            /* If the character sequence matches "true", parse a boolean true value */
        else if (wjson_match_sequence(file, L"true"))
        {
            wjson_append_boolean(wjson_node, key, true);
            return;
        }
            /* If the character sequence matches "false", parse a boolean false value */
        else if (wjson_match_sequence(file, L"false"))
        {
            wjson_append_boolean(wjson_node, key, false);
            return;
        }
            /* If the character sequence matches "null", parse a null value */
        else if (wjson_match_sequence(file, L"null"))
        {
            wjson_append_string(wjson_node, key, L"null");
            return;
        }
            /* If the character is a digit, dot, or minus sign, parse a numerical value */
        else if (iswdigit(wchar) || wchar == L'.' || wchar == L'-')
        {
            double parsedValue = wjson_parse_double(file, wchar);
            wjson_append_numerical(wjson_node, key, parsedValue);
            return;
        }
        continue;
    }
}

/**
 * @brief Parses a JSON key and its associated value from the file stream.
 *
 * This function reads characters from the file stream, parsing a key and its associated value.
 * The parsed key and value are then appended to the given wjson node.
 *
 * @param file FILE pointer to the input JSON file.
 * @param wchar Initial wide character, expected to be the opening double quote of the key.
 * @param wjson_node Pointer to the wjson node to which the parsed key and value will be appended.
 * @note The parsed key is expected to be a string, and its memory is allocated on the stack.
 * @author Toby Benjamin Clark
 */
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

/**
 * @brief Parses a JSON object from the file stream and returns a wjson pointer.
 *
 * This function reads characters from the file stream, parsing key-value pairs until the end of the object is reached.
 * Parsed key-value pairs are appended to a wjson object. The resulting wjson object is then returned.
 *
 * @param file FILE pointer to the input JSON file.
 * @return wjson pointer to the parsed JSON object.
 * @note The caller is responsible for freeing the allocated memory.
 * @author Toby Benjamin Clark
 */
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

/**
 * @brief Parses a JSON file and returns a wjson pointer representing the entire JSON structure.
 *
 * This function opens the specified JSON file, parses its contents, and returns a wjson pointer
 * representing the entire JSON structure. The caller is responsible for freeing the allocated memory.
 *
 * @param filename Name of the JSON file to parse.
 * @return wjson pointer to the parsed JSON structure.
 * @note Memory allocation failure results in an error message and program exit.
 * @author Toby Benjamin Clark
 */
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





/*
 * @brief Initializes a new wjson list instance and allocates memory for it.
 *
 * This function allocates memory for a new wjson instance, initializes its
 * members to default values (NULL, 0, false), and returns a pointer to the
 * newly created instance.
 *
 * @return Pointer to the newly initialized wjson list instance.
 * @note Memory allocation failure results in an error message and program exit.
 * @author Toby Benjamin Clark
 */
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

/*
 * @brief Appends a new string element to the wjson list.
 *
 * This function appends a new wjson node with string data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_string field is
 * updated to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param value The string value for the new element.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Update data_string to the provided value */
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

/*
 * @brief Appends a new object element to the wjson list.
 *
 * This function appends a new wjson node with object data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_object field is
 * updated to point to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param value The wjson node representing the object value.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Update data_object to point to the provided value */
    new_node->data_object = value;

    return 1;
}

/*
 * @brief Appends a new numerical element to the wjson list.
 *
 * This function appends a new wjson node with numerical data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_numerical field
 * is updated to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param value The numerical value for the new element.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Update data_numerical to the provided value */
    new_node->data_numerical = value;

    return 1;
}

/* t
 * @brief Appends a new list element to the wjson list.
 *
 * This function appends a new wjson node with list data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_list field is
 * updated to point to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param value The wjson node representing the list value.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Update data_list to point to the provided value */
    new_node->data_list = value;

    return 1;
}

/*
 * @brief Appends a new boolean element to the wjson list.
 *
 * This function appends a new wjson node with boolean data to the end of the
 * provided wjson_node list. It allocates memory for the key, sets the necessary
 * fields, and updates the list pointers accordingly. The data_bool field is
 * updated to the provided value.
 *
 * @param wjson_node The head of the wjson list to which the new element will be appended.
 * @param value The boolean value for the new element.
 * @return 1 on success, 0 on memory allocation failure.
 * @note Memory allocation failure results in an error message and returns 0.
 * @author Toby Benjamin Clark
 */
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

    /* Update data_bool to the provided value */
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