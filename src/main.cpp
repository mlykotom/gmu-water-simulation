#include <mainwindow.h>

//using non-integrated GPU 
#ifdef _WIN32
#include "windows.h"
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

int main(int, char*[])
{
    MainWindow mw;

    return mw.show();
}

