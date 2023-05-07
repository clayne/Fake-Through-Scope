#pragma once

#pragma warning(push)
#include "F4SE/F4SE.h"
#include "RE/Fallout.h"

#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif
#pragma warning(pop)
#pragma warning(disable: 4100)
#pragma warning(disable: 4189)
#pragma warning(disable: 5105)
#pragma warning(disable: 4838)

#define DLLEXPORT __declspec(dllexport)
typedef unsigned char UInt8;        //!< An unsigned 8-bit integer value
typedef unsigned short UInt16;      //!< An unsigned 16-bit integer value
typedef unsigned long UInt32;       //!< An unsigned 32-bit integer value
typedef unsigned long long UInt64;  //!< An unsigned 64-bit integer value
typedef signed char SInt8;          //!< A signed 8-bit integer value
typedef signed short SInt16;        //!< A signed 16-bit integer value
typedef signed long SInt32;         //!< A signed 32-bit integer value
typedef signed long long SInt64;    //!< A signed 64-bit integer value
typedef float Float32;              //!< A 32-bit floating point value
typedef double Float64;             //!< A 64-bit floating point value



namespace logger = F4SE::log;

using namespace std::literals;

#include "Version.h"


