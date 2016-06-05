#include "mainframe.h"

#include <iostream>

using namespace std;

void print_menu()
{
    printf("File Client Usage:\n");
    printf("N - new connection.\n");
    printf("F - file replay for connection.\n");
    printf("X - stop connection.\n");
    printf("R - restore connection by id.\n");
    printf("I - reconnecting time interval.\n");
    printf("S - sending time interval.\n");
    printf("P - packages size in bytes.\n");
    printf("M - call menu.\n");
    printf("Q - quit File Client.\n");
}

/*******************************************************/
int main()
{
    print_menu();
    try 
    {
        Mainframe mainframe;
        mainframe.join();
    }
    catch(const Exception& ex)
    {
#ifdef WIN32
        char oembuf[512] = {0};
        CharToOemBuff( ex.reason().c_str(), (LPSTR)oembuf, strlen( ex.reason().c_str() )+1 );
        printf(">> Error: %s\n", oembuf);
#else
        printf(">> Error: %s\n", ex.what());
#endif
    }
    return 0;
}
