#pragma once
#include<windows.h>
#include<sstream>
#ifdef _DEBUG
#define WARNING(flag,msg1,msg2)\
    { \
        if(flag){ \
            std::wostringstream buf; \
            buf << msg1 << '\n' << msg2 << '\n'; \
            MessageBox(0, buf.str().c_str(), L"WARNING!!", MB_OK | MB_ICONERROR); \
            exit(1); \
        } \
    }
#else
  #define WARNING(flag,msg1,msg2) {}
#endif
