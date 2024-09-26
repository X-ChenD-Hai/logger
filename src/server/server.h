// server.h
#ifndef SERVER_H
#define SERVER_H
#include "./connection.h"
#include <string>

namespace logger {
class Server {
protected:
    std::string __id;
    std::vector<Connection*> __conns;

public:
    inline std::string id() { return __id; };

    inline std::vector<Connection*> conns() { return __conns; };

public:
    Server();
    ~Server();
};

inline Server::Server()
{
}

inline Server::~Server()
{
}

} // namespace logger

#endif // SERVER_H