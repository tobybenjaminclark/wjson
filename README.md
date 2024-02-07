
# wjson Library Documentation
The wjson library is a simple C library for creating and manipulating JSON-like data structures with wide character support (wchar_t). It provides functions for creating, appending, and printing JSON objects and lists.

# Functions
##### void wjson_print(struct wjson* head, int indentation)
Recursively prints a wjson object structure with the specified indentation level.

##### struct wjson* wjson_initialize()
Initializes a new wjson object and returns a pointer to it.

##### int wjson_append_string(struct wjson* wjson_node, wchar_t* key, wchar_t* value)
Appends a string key-value pair to a wjson object.

##### int wjson_append_object(struct wjson* wjson_node, wchar_t* key, struct wjson* value)
Appends an object key-value pair to a wjson object.

##### int wjson_append_list(struct wjson* wjson_node, wchar_t* key, struct wjson* value)
Appends a list key-value pair to a wjson object.

##### int wjson_append_numerical(struct wjson* wjson_node, wchar_t* key, double value)
Appends a numerical key-value pair to a wjson object.

##### int wjson_append_boolean(struct wjson* wjson_node, wchar_t* key, bool value)
Appends a boolean key-value pair to a wjson object.

##### struct wjson* wjson_initialize_list()
Initializes a new wjson object for representing a list and returns a pointer to it.

##### int wjson_list_append_string(struct wjson* wjson_node, wchar_t* value)
Appends a string value to a wjson list.

##### int wjson_list_append_object(struct wjson* wjson_node, struct wjson* value)
Appends an object value to a wjson list.

##### int wjson_list_append_numerical(struct wjson* wjson_node, double value)
Appends a numerical value to a wjson list.

##### int wjson_list_append_list(struct wjson* wjson_node, struct wjson* value)
Appends a list value to a wjson list.

##### int wjson_list_append_boolean(struct wjson* wjson_node, bool value)
Appends a boolean value to a wjson list.

##### void wjson_test()
A sample function demonstrating the usage of the wjson library by creating, appending, and printing a complex wjson structure.