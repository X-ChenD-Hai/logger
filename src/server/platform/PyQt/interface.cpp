#include "../../ipc.hpp"
void test_ipc_() {
    logger::IpcServer s("test");
    while (1) {
        s.listening();
    }
}

int main(int argc, char const *argv[]) {
    test_ipc_();
    return 0;
}
