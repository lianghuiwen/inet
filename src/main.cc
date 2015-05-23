#include <stdio.h>
#include "logging.h"
#include "thread.h"
#include "threadpool.h"
#include "tcpserver.h"

int main(int argc, char* argv[])
{
    inet::InetAddress addr(8000);
    inet::TcpServer server(addr);
    server.Start();
}
