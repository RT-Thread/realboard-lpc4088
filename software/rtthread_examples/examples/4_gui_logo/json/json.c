/*
 * File      : json.c
 * COPYRIGHT (C) 2012-2013, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-05-05     Bernard      the first version
 */
#include <rtthread.h>
#include <JSON_parser.h>
#include <string.h>

#include "json.h"

static _json_node_destroy(struct json_node* node)
{
	int index;

	if (node->type == JSON_NODE_TYPE_STR)
	{
		if (node->vu.str_value != RT_NULL)
			json_free(node->vu.str_value);
	}

	if (node->type == JSON_NODE_TYPE_ARRAY)
	{
		for (index = 0; index < node->count; index ++)
		{
			_json_node_destroy(node->vu.array[index]);
		}
		json_free(node->vu.array);
	}

	if (node->type == JSON_NODE_TYPE_DICT)
	{
		for (index = 0; index < node->count; index ++)
		{
			_json_node_destroy(node->vu.child[index]);
		}
		json_free(node->vu.child);
	}

	if (node->key != RT_NULL)
		json_free(node->key);

	json_free(node);
}

void json_tree_destroy(struct json_tree* tree)
{
	int index;

	for (index = 0; index < tree->root.count; index ++)
	{
		_json_node_destroy(tree->root.vu.child[index]);
	}

	json_free(tree->root.vu.child);
	json_free(tree);
}

static int json_callback(void* ctx, int type, const JSON_value* value)
{
	struct json_tree* tree;
	struct json_node* node = RT_NULL;
	struct json_node* parent;
	struct json_node** items;

	tree = (struct json_tree*)ctx;
	RT_ASSERT(tree != RT_NULL);
	if (tree->layer >= 0)
	{
		node = tree->node_stack[tree->layer];
	}

	switch (type)
	{
	case JSON_T_KEY:
		parent = node;

		parent->count += 1;
		items = (struct json_node**) json_realloc (parent->vu.child, parent->count * sizeof(struct json_node*));
		if (items == RT_NULL)
		{
			rt_kprintf("out of memory\n");
			return 0;
		}
		parent->vu.child = items;

		/* create a json node */
		node = (struct json_node*) json_malloc (sizeof(struct json_node));
		/* set child node */
		parent->vu.child[parent->count - 1] = node;
		node->type = JSON_NODE_TYPE_UNKNOWN;
		node->count = 0;
		/* set key */
		node->key = rt_strdup(value->vu.str.value);
		node->vu.str_value = RT_NULL;

		tree->layer ++;
		tree->node_stack[tree->layer] = node;

		break;

	case JSON_T_STRING: /* string value */
		RT_ASSERT(node != RT_NULL);
		if (node->type == JSON_NODE_TYPE_ARRAY)
		{
			/* parent is a array */
			parent = node;

			/* create a json node */
			node = (struct json_node*) json_malloc (sizeof(struct json_node));
			node->key = RT_NULL;
			node->type = JSON_NODE_TYPE_STR;
			node->count = 0;
			/* set value */
			node->vu.str_value = rt_strdup(value->vu.str.value);

			parent->count += 1;
			items = (struct json_node**) json_realloc (parent->vu.array, parent->count * sizeof(struct json_node*));
			if (items == RT_NULL)
			{
				rt_kprintf("out of memory\n");
				return 0;
			}
			parent->vu.array = items;
			parent->vu.array[parent->count - 1] = node;
		}
		else
		{
			node->vu.str_value = rt_strdup(value->vu.str.value);
			node->type = JSON_NODE_TYPE_STR;
			tree->layer --;
		}
		break;
	case JSON_T_INTEGER: /* integer value */
		RT_ASSERT(node != RT_NULL);
		if (node->type == JSON_NODE_TYPE_ARRAY)
		{
			/* parent is a array */
			parent = node;

			/* create a json node */
			node = (struct json_node*) json_malloc (sizeof(struct json_node));
			node->key = RT_NULL;
			node->type = JSON_NODE_TYPE_INT;
			node->count = 0;
			/* set value */
			node->vu.int_value = value->vu.integer_value;

			parent->count += 1;
			items = (struct json_node**) json_realloc (parent->vu.array, parent->count * sizeof(struct json_node*));
			if (items == RT_NULL)
			{
				rt_kprintf("out of memory\n");
				return 0;
			}
			parent->vu.array = items;
			parent->vu.array[parent->count - 1] = node;
		}
		else
		{
			node->vu.int_value = value->vu.integer_value;
			node->type = JSON_NODE_TYPE_INT;
			tree->layer --;
		}
		break;
	case JSON_T_FLOAT: /* float value */
		RT_ASSERT(node != RT_NULL);
		if (node->type == JSON_NODE_TYPE_ARRAY)
		{
			/* parent is a array */
			parent = node;

			/* create a json node */
			node = (struct json_node*) json_malloc (sizeof(struct json_node));
			node->key = RT_NULL;
			node->type = JSON_NODE_TYPE_FLOAT;
			node->count = 0;
			/* set value */
			node->vu.float_value = value->vu.float_value;

			parent->count += 1;
			items = (struct json_node**) json_realloc (parent->vu.array, parent->count * sizeof(struct json_node*));
			if (items == RT_NULL)
			{
				rt_kprintf("out of memory\n");
				return 0;
			}
			parent->vu.array = items;
			parent->vu.array[parent->count - 1] = node;
		}
		else
		{
			node->vu.float_value = value->vu.float_value;
			node->type = JSON_NODE_TYPE_FLOAT;
			tree->layer --;
		}
		break;
	case JSON_T_NULL:
		RT_ASSERT(node != RT_NULL);
		node->vu.str_value = RT_NULL;
		node->type = JSON_NODE_TYPE_UNKNOWN;
		tree->layer --;
		break;
	case JSON_T_TRUE: /* boolean value */
		RT_ASSERT(node != RT_NULL);
		if (node->type == JSON_NODE_TYPE_ARRAY)
		{
			/* parent is a array */
			parent = node;

			/* create a json node */
			node = (struct json_node*) json_malloc (sizeof(struct json_node));
			node->key = RT_NULL;
			node->type = JSON_NODE_TYPE_FLOAT;
			node->count = 0;
			/* set value */
			node->vu.boolean_value = 1;

			parent->count += 1;
			items = (struct json_node**) json_realloc (parent->vu.array, parent->count * sizeof(struct json_node*));
			if (items == RT_NULL)
			{
				rt_kprintf("out of memory\n");
				return 0;
			}
			parent->vu.array = items;
			parent->vu.array[parent->count - 1] = node;
		}
		else
		{
			node->vu.boolean_value = 1;
			node->type = JSON_NODE_TYPE_BOOL;
			tree->layer --;
		}
		break;
	case JSON_T_FALSE: /* boolean value */
		RT_ASSERT(node != RT_NULL);
		if (node->type == JSON_NODE_TYPE_ARRAY)
		{
			/* parent is a array */
			parent = node;

			/* create a json node */
			node = (struct json_node*) json_malloc (sizeof(struct json_node));
			node->key = RT_NULL;
			node->type = JSON_NODE_TYPE_BOOL;
			node->count = 0;
			/* set value */
			node->vu.boolean_value = 0;

			parent->count += 1;
			items = (struct json_node**) json_realloc (parent->vu.array, parent->count * sizeof(struct json_node*));
			if (items == RT_NULL)
			{
				rt_kprintf("out of memory\n");
				return 0;
			}
			parent->vu.array = items;
			parent->vu.array[parent->count - 1] = node;
		}
		else
		{
			node->vu.boolean_value = 0;
			node->type = JSON_NODE_TYPE_BOOL;
			tree->layer --;
		}
		break;

	case JSON_T_OBJECT_BEGIN:
		if (node->type == JSON_NODE_TYPE_ARRAY)
		{
			/* parent is a array */
			parent = node;

			/* create a json node */
			node = (struct json_node*) json_malloc (sizeof(struct json_node));
			node->key = RT_NULL;
			node->type = JSON_NODE_TYPE_DICT;
			node->count = 0;
			/* set value */
			node->vu.str_value = RT_NULL;

			parent->count += 1;
			items = (struct json_node**) json_realloc (parent->vu.array, parent->count * sizeof(struct json_node*));
			if (items == RT_NULL)
			{
				rt_kprintf("out of memory\n");
				return 0;
			}
			parent->vu.array = items;
			parent->vu.array[parent->count - 1] = node;

			tree->layer += 1;
			tree->node_stack[tree->layer] = node;
		}
		else node->type = JSON_NODE_TYPE_DICT;
		break;
	case JSON_T_OBJECT_END:
		tree->layer --;
		break;

	case JSON_T_ARRAY_BEGIN:
		node->type = JSON_NODE_TYPE_ARRAY;
		break;
	case JSON_T_ARRAY_END:
		tree->layer --;
		break;

	default:
		break;
	}

	return 1;
}

static struct json_node* json_get_node_va(struct json_node* node, va_list args)
{
	const char* arg;

    if (node == RT_NULL) return RT_NULL;

	arg = (char*)va_arg(args, char*);
	while (arg != RT_NULL)
	{
		if (node->type == JSON_NODE_TYPE_DICT)
		{
			int index;
			int found = 0;

			for (index = 0; index < node->count; index ++)
			{
				if (strcmp(node->vu.child[index]->key, arg) == 0)
				{
					node = node->vu.child[index];
					arg = (char*)va_arg(args, char*);
					found = 1;
					break;
				}
			}

			/* not found */
			if (!found) return RT_NULL;
		}
		else
		{
			arg = (char*)va_arg(args, char*);
			if (arg != RT_NULL) return RT_NULL;
		}
	}

	return node;
}

struct json_node* json_get_node(struct json_node* node, ...)
{
	va_list args;
	const char* arg;

    if (node == RT_NULL) return NULL;
	va_start(args, node);

	arg = (char*)va_arg(args, char*);
	while (arg != RT_NULL)
	{
		if (node->type == JSON_NODE_TYPE_DICT)
		{
			int index;
			int found = 0;

			for (index = 0; index < node->count; index ++)
			{
				if (strcmp(node->vu.child[index]->key, arg) == 0)
				{
					node = node->vu.child[index];
					arg = (char*)va_arg(args, char*);
					found = 1;
					break;
				}
			}

			/* not found */
			if (!found)
			{
				node = RT_NULL;
				goto __exit;
			}
		}
		else
		{
			arg = (char*)va_arg(args, char*);
			if (arg != RT_NULL)
			{
				node = RT_NULL;
				goto __exit;
			}
		}
	}

__exit:
	va_end(args);

	return node;
}

struct json_tree* json_tree_parse(const char* buffer, rt_size_t length)
{
	JSON_config config;
	struct JSON_parser_struct* jc = NULL;
	struct json_tree* tree;

	const char* ptr;

	tree = (struct json_tree*) json_malloc (sizeof(struct json_tree));
	memset(tree, 0x00, sizeof(struct json_tree));
	tree->layer = 0;
	tree->root.type = JSON_NODE_TYPE_DICT;
	tree->node_stack[tree->layer] = &(tree->root);

	config.depth                  = 10;
	config.callback               = &json_callback;
	config.callback_ctx           = tree;
	config.allow_comments         = 1;
	config.handle_floats_manually = 0;

	jc = new_JSON_parser(&config);
	ptr = buffer;
	while (ptr < buffer + length)
	{
		if (!JSON_parser_char(jc, *ptr++))
		{
			rt_kprintf("JSON_parser_error: parse failed\n");
			break;
		}
	}

	if (!JSON_parser_done(jc))
	{
		rt_kprintf("JSON_parser_end: syntax error\n");
	}

	delete_JSON_parser(jc);

	return tree;
}

const char* json_node_get_string(struct json_node* node, ...)
{
	va_list args;

	RT_ASSERT(node != RT_NULL);

	va_start(args, node);
	node = json_get_node_va(node, args);
	va_end(args);

	if (node != RT_NULL && node->type == JSON_NODE_TYPE_STR) return node->vu.str_value;

	return RT_NULL;
}

int json_node_get_integer(struct json_node* node, ...)
{
	va_list args;

	RT_ASSERT(node != RT_NULL);

	va_start(args, node);
	node = json_get_node_va(node, args);
	va_end(args);

	if (node != RT_NULL && node->type == JSON_NODE_TYPE_INT) return node->vu.int_value;

	return -1;
}

float json_node_get_float(struct json_node* node, ...)
{
	va_list args;

	RT_ASSERT(node != RT_NULL);

	va_start(args, node);
	node = json_get_node_va(node, args);
	va_end(args);

	if (node != RT_NULL && node->type == JSON_NODE_TYPE_FLOAT) return (float)node->vu.float_value;

	return 0;
}

struct json_node* json_node_get_array(struct json_node* node, int index, ...)
{
	va_list args;

	RT_ASSERT(node != RT_NULL);

	va_start(args, index);
	node = json_get_node_va(node, args);
	va_end(args);

	if (node != RT_NULL && node->type == JSON_NODE_TYPE_ARRAY)
	{
		if (index >= node->count) return RT_NULL;

		return node->vu.array[index];
	}

	return RT_NULL;
}
