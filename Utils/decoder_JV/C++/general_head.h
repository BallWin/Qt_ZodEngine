#ifndef GENERAL_HEAD_H
#define GENERAL_HEAD_H
// =========================================================
// *********************************************************
// =========================================================
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <climits>
#include <memory>
#include <bitset>

#include <thread>
#include <mutex>


#ifdef WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// =========================================================
// *********************************************************
// =========================================================
#if !defined(MSG_FLINE_STR)
#define MSG_FLINE_STR(msg) std::string(             \
    "\n>>>>>------------------------>>>>>"          \
    + std::string("\n File: ")                      \
    + std::string(__FILE__)                         \
    + std::string("\n Function: ")                  \
    + std::string(&__func__[0])                     \
    + std::string("\n Line: ")                      \
    + std::to_string(__LINE__)                      \
    + std::string("\n Message:\t ")                 \
    + std::string( msg )                            \
    + "\n<<<<<-----------------------<<<<<\n"       \
    )
#endif
// =========================================================
// *********************************************************
// =========================================================
#endif // GENERAL_HEAD_H
