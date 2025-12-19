#include "db/db.hpp"
#include "engine/sstable.hpp"

#include <algorithm>
#include <sstream>
#include <ctime>

namespace mvdb {

static constexpr size_t ARENA_FACTOR = 8;

DB::DB(const std::string& dir,
       size_t memtable_bytes)
    : dir_(dir),
      mem_limit_(memtable_bytes),
      wal_(dir + "/wal.log"),
      active_(new MemTable(memtable_bytes,
                           memtable_bytes * ARENA_FACTOR)),
      immutable_(nullptr),
      next_sst_id_(0) {}

DB::~DB() {
    delete active_;
    delete immutable_;
}

void DB::put(const char* key, uint32_t key_len,
             const char* value, uint32_t value_len,
             uint64_t ttl_sec) {

    wal_.append_put(key, key_len, value, value_len, ttl_sec);
    active_->put(key, key_len, value, value_len, ttl_sec);

    maybe_flush();
}

void DB::del(const char* key, uint32_t key_len) {
    wal_.append_del(key, key_len);
    active_->del(key, key_len);

    maybe_flush();
}

bool DB::get(const char* key, uint32_t key_len,
             std::string& value_out) {

    const char* v;
    uint32_t vlen;

    // 1️⃣ Check active memtable
    if (active_->get(key, key_len, v, vlen)) {
        value_out.assign(v, vlen);
        return true;
    }

    // 2️⃣ Check immutable memtable (if exists)
    if (immutable_ && immutable_->get(key, key_len, v, vlen)) {
        value_out.assign(v, vlen);
        return true;
    }

    // 3️⃣ Check SSTables (newest → oldest)
    for (auto it = sst_files_.rbegin();
         it != sst_files_.rend(); ++it) {

        SSTable sst(*it);
        if (sst.get(key, key_len, value_out))
            return true;
    }

    return false;
}

/* ---------------- FLUSH LOGIC ---------------- */

void DB::maybe_flush() {
    if (active_->size_bytes() >= mem_limit_) {
        freeze_memtable();
        flush_immutable();
    }
}

void DB::freeze_memtable() {
    immutable_ = active_;

    active_ = new MemTable(
        mem_limit_,
        mem_limit_ * ARENA_FACTOR
    );
}

void DB::flush_immutable() {
    if (!immutable_) return;

    std::vector<std::pair<std::string, std::string>> kvs;

    immutable_->for_each([&](const Entry& e) {
        if (e.expire_ts != 0 &&
            e.expire_ts < (uint64_t)std::time(nullptr))
            return;

        kvs.emplace_back(
            std::string(e.key, e.key_len),
            std::string(e.value, e.value_len)
        );
    });

    std::sort(kvs.begin(), kvs.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });

    std::ostringstream ss;
    ss << dir_ << "/sst_" << next_sst_id_++ << ".sst";

    SSTable::build(ss.str(), kvs);
    sst_files_.push_back(ss.str());

    delete immutable_;
    immutable_ = nullptr;
}

} // namespace mvdb
