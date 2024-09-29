#include <iostream>
#include <thread>

#include "../../ipc.hpp"

void test_ipc_() {
    using namespace std;
    logger::IpcServer s("test");
    bool stop = false;
    std::thread t([&s, &stop]() {
        while (!stop) {
            s.listening();
            if (stop) std::cout << "Stop" << endl;
        }
    });
    std::this_thread::sleep_for(5s);
    stop = true;
    t.join();
    for (auto con : s.connections()) {
        cout << con->ClientName << endl;
    }
}

int main(int argc, char const *argv[]) {
    test_ipc_();
    return 0;
}
