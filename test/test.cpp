#include <thread>

#include "../src/client/client.hpp"
#include "../src/client/ipc.hpp"
#include "libipc/ipc.h"

using namespace std;

void test_ipc() {
    std::vector<char const *> const datas = {
        "hello!",
        "foo",
        "bar",
        "ISO/IEC",
        "14882:2011",
        "ISO/IEC 14882:2017 Information technology - Programming languages - ",
        "C++",
        "ISO/IEC 14882:2020",
        "Modern C++ Design: Generic Programming and Design Patterns Applied"};

    // thread producer
    std::thread t1{[&] {
        ipc::route cc{"my-ipc-route"};
        // waiting for connection
        cc.wait_for_recv(1);
        // sending datas
        for (auto str : datas) cc.send(str);
        // quit
        cc.send(ipc::buff_t('\0'));
    }};

    // thread consumer

    std::thread t2{[&] {
        ipc::route cc{"my-ipc-route", ipc::receiver};
        while (1) {
            auto buf = cc.recv();
            auto str = static_cast<char *>(buf.data());
            if (str == nullptr || str[0] == '\0') return;
            std::printf("recv: %s\n", str);
        }
    }};

    t1.join();
    t2.join();
}

int main(int argc, char const *argv[]) {
    auto client =
        logger::getClient("test", {new logger::IpcConnention("test")});
    auto root = client->root();
    auto t1 = root->child("test", {"hello", "Hello2"});
    t1->log(1, "hello world", {"hello", "world"});

    this_thread::sleep_for(5s);
    return 0;
}
