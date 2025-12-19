#include "db/db.hpp"
#include <iostream>
#include <string>

using namespace mvdb;
using namespace std;
int main() {
    DB db("testdb", 64*1024*1024); // small memtable to test flush too

    std::string cmd;
    while (true) {
        std::cout << "> ";
        std::cin >> cmd;

        if (cmd == "put") {
            int z=1e6;
            for(int i=0;i<z;i++) {
                string k="key"+to_string(i);
                string v="val_"+to_string(i);
                db.put(k.data(),k.size(),v.data(),v.size());
            }
            std::string k, v;
            std::cin >> k >> v;
            db.put(k.data(),k.size(),v.data(),v.size());
            std::cout << "OK\n";
        }
        else if (cmd == "get") {
            std::string k;
            std::cin >> k;
            std::string v;
            if (db.get(k.data(), k.size(), v))
                std::cout << v << "\n";
            else
                std::cout << "(not found)\n";
        }
        else if (cmd == "del") {
            std::string k;
            std::cin >> k;
            db.del(k.data(), k.size());
            std::cout << "OK\n";
        }
        else if (cmd == "exit") {
            break;
        }
        else {
            std::cout << "Commands: put <k> <v>, get <k>, del <k>, exit\n";
        }
    }

    return 0;
}
