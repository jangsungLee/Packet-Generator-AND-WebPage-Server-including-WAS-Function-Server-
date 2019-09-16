#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
 #define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include "WebSocket_Encoding_HS.h"
#include "packet_generator.h"


#pragma comment (lib, "ws2_32.lib")

extern "C" __declspec(dllexport) void OnTest1();
extern "C" __declspec(dllexport) void shutdownWAS();
// extern "C" __declspec(dllexport) void Timeout();
// --> 아무런 동작을 하지 않고 연결만 유지하는 클라이언트에 대해 Ping메세지를 보내고 응답을 받지 않으면 종료
// 요청을 할 때마다 시간을 초기화 해서 비교
// 아무런 동작을 하지 않은채 Timeout 3회 이상시간을 넘어가면 연결을 종료시킴
// 중요한 이유 : 메모리용량 + 이용자수를 공평하고 많이 수용하기 위해서 만들어야함

// test.h
//extern "C" __declspec(dllexport) char* _GetAdapterDescription(int* size, UINT index/*0 : reallocate, >0 : index pointer*/)
//extern "C" __declspec(dllexport) int _SetAdapter(char ptr[133])