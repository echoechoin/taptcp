#include <stdio.h>
#include <stdlib.h>

#ifdef TUN_SERVER
#include "server.h"
int main(int argc, char **argv) 
{
    return run_server(argc, argv);
}

#else

#include "client.h"
int main(int argc, char **argv) 
{
    return run_client(argc, argv);
}

#endif
