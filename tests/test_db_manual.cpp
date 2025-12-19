#include "db/db.hpp"
#include <iostream>
#include <string>

using namespace mvdb;

int main() {
    DB db("testdb", 64 * 1024 * 1024); // small memtable to test flush too

    std::string cmd;
    while (true) {
        std::cout << "> ";
        std::cin >> cmd;

        if (cmd == "put") {
            std::string k, v;
            std::cin >> k >> v;
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
