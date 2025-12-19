#pragma once
#include <cstdint>
#include <cstddef>

namespace mvdb {

class Arena;

/* Entry state for open addressing */
enum class EntryState : uint8_t {
    EMPTY = 0,
    OCCUPIED = 1,
    TOMBSTONE = 2
};

struct Entry {
    uint64_t hash;
    const char* key;
    uint32_t key_len;

    const char* value;
    uint32_t value_len;

    uint64_t expire_ts;   // 0 = no TTL
    EntryState state;
};

class HashTable {
public:
    HashTable(size_t initial_capacity, Arena* arena);
    ~HashTable();

    bool put(const char* key, uint32_t key_len,
             const char* value, uint32_t value_len,
             uint64_t expire_ts = 0);

    bool get(const char* key, uint32_t key_len,
             const char*& value_out, uint32_t& value_len_out);

    bool del(const char* key, uint32_t key_len);

    size_t size() const { return size_; } 

private:
    void rehash();
    size_t find_slot(uint64_t hash,
                     const char* key,
                     uint32_t key_len,
                     bool for_insert);

    uint64_t hash_key(const char* key, uint32_t len) const;

private:
    Entry* entries_;
    size_t capacity_;
    size_t size_;
    size_t tombstones_;
    Arena* arena_;
};

} // namespace mvdb
