// server.h
#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H
namespace logger {

class ServerManager {
protected:
    bool __stop = false;

public:
    ServerManager();
    ~ServerManager() {};
};

};

#endif // SERVER_MANAGER_H