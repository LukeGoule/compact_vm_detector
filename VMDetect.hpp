#pragma once

#include <iostream>
#include <Windows.h>
#include <intrin.h>
#include <vector>
#include <fstream>

#define STATUS_HV_DETECTED 1
#define STATUS_HV_NOT_PRESENT 0
#define HV_SYNTHETIC_MSR_RANGE_START 0x40000000

namespace VMDetect
{
#if _WIN32 && !(_WIN64)
	DWORD64 CalcRDTSCDifference();
#endif

#if _WIN64
	DWORD64 CalcRDTSCDifference();
#endif

	BOOLEAN CheckRTDSCTimings(VOID);
	BOOLEAN CheckCPUIDLeafResponse(VOID);
	BOOLEAN CheckSyntheticMSRs(VOID);

	BOOLEAN FileExists(std::string FileName);
	BOOLEAN CheckDebuggerPresent(VOID);

	namespace VirtualBox
	{
		BOOLEAN CheckSystemFiles(VOID);
		BOOLEAN CheckTrayWindowExists(VOID);
	}

	namespace CPU
	{
		BOOLEAN CheckCpuVendor(VOID);
		BOOLEAN CheckBochsAMD(VOID);
		BOOLEAN CheckHVBit(VOID);
	}
}