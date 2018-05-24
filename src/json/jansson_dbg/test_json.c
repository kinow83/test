/*
 * Simple example of parsing and printing JSON using jansson.
 *
 * SYNOPSIS:
 * $ examples/simple_parse
 * Type some JSON > [true, false, null, 1, 0.0, -0.0, "", {"name": "barney"}]
 * JSON Array of 8 elements:
 *   JSON True
 *   JSON False
 *   JSON Null
 *   JSON Integer: "1"
 *   JSON Real: 0.000000
 *   JSON Real: -0.000000
 *   JSON String: ""
 *   JSON Object of 1 pair:
 *     JSON Key: "name"
 *     JSON String: "barney"
 *
 * Copyright (c) 2014 Robert Poor <rdpoor@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

/* forward refs */
void print_json(json_t *root);
void print_json_aux(json_t *element, int indent);
void print_json_indent(int indent);
const char *json_plural(int count);
void print_json_object(json_t *element, int indent);
void print_json_array(json_t *element, int indent);
void print_json_string(json_t *element, int indent);
void print_json_integer(json_t *element, int indent);
void print_json_real(json_t *element, int indent);
void print_json_true(json_t *element, int indent);
void print_json_false(json_t *element, int indent);
void print_json_null(json_t *element, int indent);

#if 0
static int cnt = 0;

static void json_release(json_t *element, int indent) {
	size_t index;
    json_t *value;
    const char *key;

    switch (json_typeof(element)) {
    case JSON_OBJECT:
		printf("free obj:%d [%d]\n", cnt++, indent);
		json_object_foreach(element, key, value) {
			json_release(value, indent+1);
		}
        break;
    case JSON_ARRAY:
		printf("free arr:%d [%d]\n", cnt++, indent);
		json_array_foreach(element, index, value) {
			json_release(value, indent+1);
		}
        break;
    case JSON_STRING:
    case JSON_INTEGER:
    case JSON_REAL:
    case JSON_TRUE:
    case JSON_FALSE:
    case JSON_NULL:
		json_decref(element);
		printf("free val:%d [%d]\n", cnt++, indent);
        break;
    }
	printf("--> ref:%d\n", element->refcount);
}
#endif

void print_json(json_t *root) {
    print_json_aux(root, 0);
}

void print_json_aux(json_t *element, int indent) {
    switch (json_typeof(element)) {
    case JSON_OBJECT:
        print_json_object(element, indent);
        break;
    case JSON_ARRAY:
        print_json_array(element, indent);
        break;
    case JSON_STRING:
        print_json_string(element, indent);
        break;
    case JSON_INTEGER:
        print_json_integer(element, indent);
        break;
    case JSON_REAL:
        print_json_real(element, indent);
        break;
    case JSON_TRUE:
        print_json_true(element, indent);
        break;
    case JSON_FALSE:
        print_json_false(element, indent);
        break;
    case JSON_NULL:
        print_json_null(element, indent);
        break;
    default:
        fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(element));
    }
}

void print_json_indent(int indent) {
    int i;
    for (i = 0; i < indent; i++) { putchar(' '); }
}

const char *json_plural(int count) {
    return count == 1 ? "" : "s";
}

void print_json_object(json_t *element, int indent) {
    size_t size;
    const char *key;
    json_t *value;

    print_json_indent(indent);
    size = json_object_size(element);

    printf("JSON Object of %ld pair%s:\n", size, json_plural(size));
    json_object_foreach(element, key, value) {
        print_json_indent(indent + 2);
        printf("JSON Key: \"%s\"\n", key);
        print_json_aux(value, indent + 2);
    }

}

void print_json_array(json_t *element, int indent) {
    size_t i;
    size_t size = json_array_size(element);
    print_json_indent(indent);

    printf("JSON Array of %ld element%s:\n", size, json_plural(size));
    for (i = 0; i < size; i++) {
        print_json_aux(json_array_get(element, i), indent + 2);
    }
}

void print_json_string(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON String: \"%s\"\n", json_string_value(element));
}

void print_json_integer(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
}

void print_json_real(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON Real: %f\n", json_real_value(element));
}

void print_json_true(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON True\n");
}

void print_json_false(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON False\n");
}

void print_json_null(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON Null\n");
}

/*
 * Parse text into a JSON object. If text is valid JSON, returns a
 * json_t structure, otherwise prints and error and returns null.
 */
json_t *load_json(const char *text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if (root) {
        return root;
    } else {
        fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
        return (json_t *)0;
    }
}

/*
 * Print a prompt and return (by reference) a null-terminated line of
 * text.  Returns NULL on eof or some error.
 */
char *read_line(char *line, int max_chars) {
    printf("Type some JSON > ");
    fflush(stdout);
    return fgets(line, max_chars, stdin);
}

/* ================================================================
 * main
 */

#define MAX_CHARS 4096

#define MM 1000000
static void *mm[MM];
static int mcnt = 0;
static void *my_malloc(size_t s)
{
	if (s == 0) return NULL;
#if 0
	mm[mcnt] = malloc(s);
	return mm[mcnt++];
#else
	return malloc(s);
#endif
}
static void my_free(void *s)
{
	int i;
#if 0
	for (i=0; i<mcnt; i++) {
		if (s == mm[i]) {
			mm[i] = NULL;
		}
	}
#endif
	if (s) {
		free(s);
	}
	s = NULL;
}
static void my_init()
{
	int i;
	for (i=0; i<MM; i++) {
		mm[i] = NULL;
	}
}
static void my_all_free()
{
	int i;
	for (i=0; i<mcnt; i++) {
		if (mm[i] == NULL) continue;
		printf("free:%p\n", mm[i]);
		free(mm[i]);
		mm[i] = NULL;
	}
	for (;i<MM; i++) {
		mm[i] = NULL;
	}
	printf("%d freed\n", mcnt);
}

void** foo1()
{
	int i;
	void **pp = malloc(sizeof(void*)*MM);
	for (i=0; i<MM; i++) {
		pp[i] = my_malloc(i+10 % 25);
	}
	return pp;
}

void foo2(void **pp)
{
	int i;
	for (i=0; i<MM; i++) {
		my_free(pp[i]);
	}
	free(pp);
}

int main(int argc, char *argv[]) {
#if 1
	json_t *root;
	json_t *summary;
	json_t *api;
	json_t *name, *url;
	json_t *entrys;
	FILE *fp;
	json_error_t error;

	my_init();

	json_set_alloc_funcs(my_malloc, my_free);

	printf("load\n");
	getchar();

	fp = fopen(argv[1], "r");
	root = json_loadf(fp, 0xff, &error);
	if (!root) {
		printf("%s\n", error.text);
		exit(1);
	}
	fclose(fp);

	summary = json_object_get(root, "summary");
#if 0
	api = json_object_get(summary, "api");
	if (!api) { exit(1); }
	name = json_object_get(api, "name");
	url  = json_object_get(api, "url");
	printf("name:%s\n", json_string_value(name));
	printf("url:%s\n", json_string_value(url));
    print_json(root);
#endif


	entrys = json_object_get(summary, "entrys");
	printf("%ld\n", json_array_size(entrys));

	printf("decref\n");
	getchar();
#if 0
	for (int i=0; i<json_array_size(entrys); i++) {
		json_array_remove(entrys, i);
	}
#endif
	json_array_clear(entrys);
	printf("clear ok\n");
	json_decref(entrys);
	printf("array decref ok\n");
	json_decref(root);
	printf("root decref ok\n");
	my_all_free();
	root = NULL;

	printf("end\n");
	getchar();
#else
	printf("ready\n");
	void **pp;

	printf("malloc\n");
	getchar();
	pp = foo1();

	printf("release\n");
	getchar();
	foo2(pp);

	printf("end\n");
	getchar();
#endif

    return 0;
}
