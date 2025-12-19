#include "db/db.hpp"
#include "engine/sstable.hpp"

#include <algorithm>
#include <sstream>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

namespace mvdb {

static constexpr size_t ARENA_FACTOR = 8;

/* ================= CONSTRUCTOR / DESTRUCTOR ================= */

DB::DB(const std::string& dir,
       size_t memtable_bytes)
    : dir_(dir),
      mem_limit_(memtable_bytes),
      wal_(dir + "/wal.log"),
      active_(new MemTable(memtable_bytes,
                           memtable_bytes * ARENA_FACTOR)),
      immutable_(nullptr),
      next_sst_id_(0) {

    load_sstables();   // load existing SSTables
    replay_wal();      // rebuild MemTable from WAL
}

DB::~DB() {
    delete active_;
    delete immutable_;
}

/* ================= WRITE PATH ================= */

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

/* ================= READ PATH ================= */

bool DB::get(const char* key, uint32_t key_len,
             std::string& value_out) {

    const char* v;
    uint32_t vlen;

    // 1️⃣ Active MemTable
    if (active_->get(key, key_len, v, vlen)) {
        value_out.assign(v, vlen);
        return true;
    }

    // 2️⃣ Immutable MemTable (if flushing)
    if (immutable_ && immutable_->get(key, key_len, v, vlen)) {
        value_out.assign(v, vlen);
        return true;
    }

    // 3️⃣ SSTables (newest → oldest)
    for (auto it = sst_files_.rbegin();
         it != sst_files_.rend(); ++it) {

        SSTable sst(*it);
        if (sst.get(key, key_len, value_out))
            return true;
    }

    return false;
}

/* ================= FLUSH LOGIC ================= */

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

/* ================= RECOVERY ================= */

void DB::load_sstables() {
    if (!fs::exists(dir_)) return;

    for (const auto& entry : fs::directory_iterator(dir_)) {
        if (!entry.is_regular_file()) continue;

        const auto& path = entry.path();
        if (path.extension() == ".sst") {
            sst_files_.push_back(path.string());

            // extract sst id
            std::string stem = path.stem().string(); // sst_X
            auto pos = stem.find('_');
            if (pos != std::string::npos) {
                uint64_t id = std::stoull(stem.substr(pos + 1));
                next_sst_id_ = std::max(next_sst_id_, id + 1);
            }
        }
    }

    std::sort(sst_files_.begin(), sst_files_.end());
}

void DB::replay_wal() {
    wal_.replay(*active_);
}

} // namespace mvdb
