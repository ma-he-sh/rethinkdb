#ifndef __SERVER_NESTED_DEMO_REDIS_UTILS_HPP_
#define	__SERVER_NESTED_DEMO_REDIS_UTILS_HPP_

#include "btree/operations.hpp"

/* nested string value type */
// TODO! Replace this by a version that uses blobs!
struct redis_nested_string_value_t {
    int length;
    char contents[];

public:
    int inline_size(UNUSED block_size_t bs) const {
        return sizeof(length) + length;
    }

    int64_t value_size() const {
        return length;
    }

    const char *value_ref() const { return contents; }
    char *value_ref() { return contents; }
} __attribute__((__packed__));
template <>
class value_sizer_t<redis_nested_string_value_t> {
public:
    value_sizer_t<redis_nested_string_value_t>(block_size_t bs) : block_size_(bs) { }

    int size(const redis_nested_string_value_t *value) const {
        return value->inline_size(block_size_);
    }

    bool fits(UNUSED const redis_nested_string_value_t *value, UNUSED int length_available) const {
        // TODO!
        return size(value) <= length_available;
    }

    int max_possible_size() const {
        // TODO?
        return MAX_BTREE_VALUE_SIZE;
    }

    static block_magic_t btree_leaf_magic() {
        block_magic_t magic = { { 'l', 'r', 'n', 's' } };  // leaf redis nested string
        return magic;
    }

    block_size_t block_size() const { return block_size_; }

protected:
    // The block size.  It's convenient for leaf node code and for
    // some subclasses, too.
    block_size_t block_size_;
};


/* nested empty value type */
struct redis_nested_empty_value_t {

} __attribute__((__packed__));
template <>
class value_sizer_t<redis_nested_empty_value_t> {
public:
    value_sizer_t<redis_nested_empty_value_t>(block_size_t bs) : block_size_(bs) { }

    int size(UNUSED const redis_nested_empty_value_t *value) const {
        return 0;
    }

    bool fits(UNUSED const redis_nested_empty_value_t *value, UNUSED int length_available) const {
        return true;
    }

    int max_possible_size() const {
        return 0;
    }

    static block_magic_t btree_leaf_magic() {
        block_magic_t magic = { { 'l', 'r', 'n', 'e' } }; // leaf redis nested empty
        return magic;
    }

    block_size_t block_size() const { return block_size_; }

protected:
    // The block size.  It's convenient for leaf node code and for
    // some subclasses, too.
    block_size_t block_size_;
};

namespace redis_utils {
    /* Constructs a btree_key_t from an std::string and puts it into out_buf */
    void construct_key(const std::string &key, scoped_malloc<btree_key_t> *out_buf);

    /* Convenience accessor functions for nested trees */
    template<typename Value> void find_nested_keyvalue_location_for_write(block_size_t block_size, boost::shared_ptr<transaction_t> &transaction, const std::string &field, repli_timestamp_t ts, keyvalue_location_t<Value> *kv_location, scoped_malloc<btree_key_t> *btree_key, const block_id_t nested_root) {
        boost::scoped_ptr<superblock_t> nested_btree_sb(new virtual_superblock_t(nested_root));

        // Construct a sizer for the sub tree, using the same block size as the super tree
        value_sizer_t<Value> sizer(block_size);

        got_superblock_t got_superblock;
        got_superblock.sb.swap(nested_btree_sb);
        got_superblock.txn = transaction;

        // Construct the key
        construct_key(field, btree_key);

        // Find the element
        ::find_keyvalue_location_for_write(&sizer, &got_superblock, btree_key->get(), ts, kv_location);
    }
    template<typename Value> void find_nested_keyvalue_location_for_read(block_size_t block_size, boost::shared_ptr<transaction_t> &transaction, const std::string &field, keyvalue_location_t<Value> *kv_location, const block_id_t nested_root) {
        boost::scoped_ptr<superblock_t> nested_btree_sb(new virtual_superblock_t(nested_root));

        // Construct a sizer for the sub tree, using the same block size as the super tree
        value_sizer_t<Value> sizer(block_size);

        got_superblock_t got_superblock;
        got_superblock.sb.swap(nested_btree_sb);
        got_superblock.txn = transaction;

        // Construct the key
        scoped_malloc<btree_key_t> btree_key;
        construct_key(field, &btree_key);

        // Find the element
        ::find_keyvalue_location_for_read(&sizer, &got_superblock, btree_key.get(), kv_location);
    }

    /* Conversion between int and a lexicographical string representation */
    const size_t LEX_INT_SIZE = sizeof(int);
    void to_lex_int(const int i, char *buf);
    int from_lex_int(const char *buf);

    /* Conversion between float and a lexicographical string representation */
    /* Note: In the lexicographic representation, it is -0.0f < 0.0f. */
    const size_t LEX_FLOAT_SIZE = LEX_INT_SIZE;
    void to_lex_float(const float f, char *buf);
    float from_lex_float(const char *buf);
} /* namespace redis_utils */

#endif	/* __SERVER_NESTED_DEMO_REDIS_UTILS_HPP_ */

