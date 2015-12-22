/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Generic ESP8266 Module, Platform=esp8266, Package=esp8266com
*/

#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
#define __ESP8266_ESp8266__
#define __ESP8266_ESP8266__
#define __ets__
#define ICACHE_FLASH
#define F_CPU 80000000L
#define ARDUINO 10605
#define ARDUINO_ESP8266_ESP01
#define ARDUINO_ARCH_ESP8266
#define __cplusplus
#define __ARM__
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
typedef int __gnuc_va_list;

#define __ICCARM__
#define __ASM
#define __INLINE
#define __attribute__(noinline)

#define _STD_BEGIN
#define EMIT
#define WARNING
#define _Lockit
#define __CLR_OR_THIS_CALL
#define C4005

typedef int uint8_t;
#define __ARMCC_VERSION 400678
#define PROGMEM
#define string_literal

#define prog_void
#define PGM_VOID_P int

#define _GLIBCXX_CONSTEXPR  ([=] () {int _a = (1), _b = (2); return _a > _b ? _a : _b; }())


typedef int _read;
typedef int _seek;
typedef int _write;
typedef int _close;
typedef int __cleanup;

#define inline 
#define __builtin_clz
#define __CHAR_BIT__
#define _EXFUN()
#define __builtin_labs

//MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
//MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
//MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
//MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
//#if (_MSC_VER == 1600)
//	#undef __cplusplus
//#endif

typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}



#include <arduino.h>
#include <pins_arduino.h> 
#undef F
#define F(string_literal) ((const PROGMEM char *)(string_literal))
#undef PSTR
#define PSTR(string_literal) ((const PROGMEM char *)(string_literal))
#undef cli
#define cli()
#define pgm_read_byte(address_short)
#define pgm_read_word(address_short)
#define pgm_read_word2(address_short)
#define digitalPinToPort(P)
#define digitalPinToBitMask(P) 
#define digitalPinToTimer(P)
#define analogInPinToBit(P)
#define portOutputRegister(P)
#define portInputRegister(P)
#define portModeRegister(P)
#include <..\NTPClient\NtpClient.ino>
#include <src\_micro-api\libraries\NtpClientLib\src\NtpClientLib.cpp>
#include <src\_micro-api\libraries\NtpClientLib\src\NtpClientLib.h>
#endif
