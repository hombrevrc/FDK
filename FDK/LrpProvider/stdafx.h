#ifdef _MSC_VER
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma warning (disable : 4290)
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Wspiapi.h>



#include <string>
#include <map>
#include <stack>
#include <vector>
#include <numeric>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <memory.h>
#include <memory>
#include <exception>
#include <stdexcept>
#include <limits>
#include <sstream>
#include <regex>
#include <iomanip>

using namespace std;

#define FX_OVERRIDE_WINSOCKS
#include "../Sal/Sal.h"
#include "../Sal/Threading.h"


#pragma warning (push)
#pragma warning (disable : 4251)
#include "../Core/Core.h"
#pragma warning (pop)
// TODO: reference additional headers your program requires here


#define HAVE_CUSTOM_DATE_TIME
#include "../../External/Include/lrp/Lrp.Core.h"


#ifdef _MSC_VER
#ifdef _DEBUG
#include <crtdbg.h>
#define new new (_CLIENT_BLOCK, __FILE__, __LINE__)
#endif
#endif


typedef __int32 int32;
typedef unsigned __int32 uint32;


#ifdef max
#undef max
#endif


#ifdef min
#undef min
#endif



#include "../LrpCommon/LrpCommon.h"


