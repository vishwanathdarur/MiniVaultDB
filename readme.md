minivaultdb/
├── include/
│   ├── db/
│   │   ├── db.h              # C API
│   │   ├── options.h
│   │   └── status.h
│   ├── engine/
│   │   ├── memtable.h
│   │   ├── wal.h
│   │   ├── sstable.h
│   │   ├── compaction.h
│   │   ├── cache.h
│   │   └── recovery.h
│   └── util/
│       ├── crc32.h
│       ├── arena.h
│       ├── hash.h
│       └── file.h
│
├── src/
│   ├── db/
│   │   └── db.cc              # DB glue layer
│   │
│   ├── engine/
│   │   ├── memtable.cc
│   │   ├── wal.cc
│   │   ├── sstable.cc
│   │   ├── compaction.cc
│   │   ├── cache.cc
│   │   └── recovery.cc
│   │
│   ├── util/
│   │   ├── crc32.cc
│   │   ├── arena.cc
│   │   ├── hash.cc
│   │   └── file_posix.cc
│   │
│   └── c_api/
│       └── c_api.cc
│
├── python/
│   ├── minivaultdb/
│   │   └── __init__.py
│   └── bindings.cpp           # pybind11 / ctypes
│
├── tests/
│   ├── test_memtable.cpp
│   ├── test_wal.cpp
│   ├── test_sstable.cpp
│   ├── test_compaction.cpp
│   └── fault_injection/
│
├── bench/
│   ├── bench_put.cc
│   ├── bench_get.cc
│   └── sqlite_compare.py
│
├── tools/
│   └── sstable_dump.cpp
│
├── CMakeLists.txt
└── README.md

        
# MiniValult DB

    - Log structure merge based key value database

# Core Components

## 1. MemTable

- The **MemTable** is an **in-memory data structure** (RAM), usually implemented as a:
  - Skip List  
  - Balanced Tree (e.g., Red-Black Tree)

- It stores **recent writes (key–value pairs)**.
- **All writes go to the MemTable first**.

- When the MemTable reaches a **size limit**:
  - It becomes **immutable**
  - It is **flushed to disk** as an **SSTable**
---

## 2. SSTable (Sorted String Table)

- An **SSTable** is an **immutable, sorted file** stored on disk.
- Created when a **MemTable is flushed**.

### SSTable Contains
- Sorted key–value pairs
- Index
- Bloom filter (usually)

- **Multiple SSTables** can exist at the same time.
- During reads, the system may need to check **multiple SSTables**  
  *(from newest to oldest)*.

---

## 3. WAL (Write-Ahead Log)

- **WAL** is an **append-only log** stored on disk.
- For every write:
  1. Write to WAL
  2. Write to MemTable

### Purpose
- **Crash recovery**

- If the system crashes:
  - WAL is **replayed** to rebuild the MemTable

- Once a MemTable is flushed to an SSTable:
  - Corresponding WAL entries can be **discarded**

---

## Summary

- **MemTable** → In-memory write buffer  
- **SSTable** → Immutable sorted disk file  
- **WAL** → Crash recovery log
## MemTable
    - its kind of ram in the laptop
    - stores the present database
    - everytime it load one sstable(holds one sstable)
## SSTable
    - its like structured date stored in disc of a computer
    - whenever Memtable is full it create sstable and write on a disc
## wal
    - it recover crash
