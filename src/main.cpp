#include<unistd.h>
#include"server/webserver.h"

int main()
{
    WebServer server(
        12345,3,60000,false,
        3306,"root","Helix0819!","web_demo",
        12,6,true,1,1024
    );
    server.Start();
}