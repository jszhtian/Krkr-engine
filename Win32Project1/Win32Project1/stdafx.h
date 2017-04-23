// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�: 
#include <windows.h>
#include <cstdio>
#include <string.h>

#define TJS_BS_SEEK_SET 0
#define TJS_BS_SEEK_CUR 1
#define TJS_BS_SEEK_END 2
#define TJS_INTF_METHOD __cdecl
#define TJS_UI64_VAL(x) ((tjs_uint64)(x##i64))
#define TJS_BS_ACCESS_MASK 0x0f
#define TJS_BS_READ 0
#define TJS_BS_WRITE 1
#define TJS_BS_APPEND 2
#define TJS_BS_UPDATE 3
typedef __int8 tjs_int8;
typedef unsigned __int8 tjs_uint8;
typedef __int16 tjs_int16;
typedef unsigned __int16 tjs_uint16;
typedef __int32 tjs_int32;
typedef unsigned __int32 tjs_uint32;
typedef __int64 tjs_int64;
typedef unsigned __int64 tjs_uint64;
typedef int tjs_int;    /* at least 32bits */
typedef unsigned int tjs_uint;    /* at least 32bits */
typedef wchar_t tjs_char;



// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�