#ifndef __BTREE_DETEMPLATIZER_HPP__
#define	__BTREE_DETEMPLATIZER_HPP__

// TODO! Uhm, refactor the sizer definitions to a central place.
#include "server/nested_demo/redis_utils.hpp"
#include "server/nested_demo/redis_sortedset_values.hpp"

/*
 op_name is the name of a template function to call, arguments are the function
 arguments, and leaf_node is a pointer to a leaf_node_t.
 DETEMPLATIZE_LEAF_NODE_OP(op_name, leaf_node, sizer_argument, ...)
 selects a typename T based on the leaf_node's magic and runs the following:
 value_sizer_t<T>(sizer_argument) sizer;
 op_name<T>(&sizer, ...);
 */
#define DETEMPLATIZE_LEAF_NODE_OP(op_name, leaf_node, sizer_argument, ...) \
    do { \
        if (leaf_node->magic == value_sizer_t<memcached_value_t>::btree_leaf_magic()) { \
            value_sizer_t<memcached_value_t> sizer(sizer_argument); \
            op_name<memcached_value_t>(&sizer, __VA_ARGS__); \
        } else if (leaf_node->magic == value_sizer_t<redis_nested_string_value_t>::btree_leaf_magic()) { \
            value_sizer_t<redis_nested_string_value_t> sizer(sizer_argument); \
            op_name<redis_nested_string_value_t>(&sizer, __VA_ARGS__); \
        } else if (leaf_node->magic == value_sizer_t<redis_nested_empty_value_t>::btree_leaf_magic()) { \
            value_sizer_t<redis_nested_empty_value_t> sizer(sizer_argument); \
            op_name<redis_nested_empty_value_t>(&sizer, __VA_ARGS__); \
        } else if (leaf_node->magic == value_sizer_t<redis_nested_float_value_t>::btree_leaf_magic()) { \
            value_sizer_t<redis_nested_float_value_t> sizer(sizer_argument); \
            op_name<redis_nested_float_value_t>(&sizer, __VA_ARGS__); \
        } else \
            crash("Unmatched leaf node magic: %.*s", (int)sizeof(block_magic_t), leaf_node->magic.bytes); \
    } while(0)

#endif	/* __BTREE_DETEMPLATIZER_HPP__ */

