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
