#include "VMDetect.hpp"
#include "InstructionSet_MS.hpp"


#if _WIN32 && !(_WIN64)
// Inline assembly for possible quicker execution.
DWORD64 VMDetect::CalcRDTSCDifference()
{
	unsigned long long cycles_one, cycles_two;
	unsigned cycles_high, cycles_low;

	__asm
	{
		RDTSC
		MOV cycles_high, eax
		MOV cycles_low, edx
	}

	cycles_one = ((unsigned long long)cycles_high) | (((unsigned long long)cycles_low) << 32);

	__asm
	{
		RDTSC
		MOV cycles_high, eax
		MOV cycles_low, edx
	}

	cycles_two = ((unsigned long long)cycles_high) | (((unsigned long long)cycles_low) << 32);

	return cycles_two - cycles_one;
}
#endif

#if _WIN64

// Inline assembly is not available when compiling using 64 bit.
DWORD64 VMDetect::CalcRDTSCDifference()
{
	DWORD64 dwCyclesFirst = ReadTimeStampCounter();
	DWORD64 dwCyclesSecond = ReadTimeStampCounter();

	return dwCyclesSecond - dwCyclesFirst;
}

#endif
BOOLEAN VMDetect::CheckRTDSCTimings(VOID)
{
	unsigned long long i = 0, average = 0;

	for (; i < 10; i++)
	{
		average += CalcRDTSCDifference();
		Sleep(500);
	}

	average /= i;

	return (average < 750 && average > 0) ? FALSE : TRUE;
}

BOOLEAN VMDetect::CheckCPUIDLeafResponse(VOID)
{
	unsigned int invalid_leaf = 0x13371337;
	unsigned int valid_leaf = 0x40000000;

	struct _HV_DETAILS
	{
		unsigned int Data[4];
	};

	_HV_DETAILS InvalidLeafResponse = { 0 };
	_HV_DETAILS ValidLeafResponse = { 0 };

	__cpuid((int*)&InvalidLeafResponse, invalid_leaf);
	__cpuid((int*)&ValidLeafResponse, valid_leaf);

	if ((InvalidLeafResponse.Data[0] != ValidLeafResponse.Data[0]) ||
		(InvalidLeafResponse.Data[1] != ValidLeafResponse.Data[1]) ||
		(InvalidLeafResponse.Data[2] != ValidLeafResponse.Data[2]) ||
		(InvalidLeafResponse.Data[3] != ValidLeafResponse.Data[3]))
		return TRUE;

	return FALSE;
}

BOOLEAN VMDetect::CheckSyntheticMSRs(VOID)
{
	__try
	{
		__readmsr(HV_SYNTHETIC_MSR_RANGE_START);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE;
	}

	return TRUE;
}

BOOLEAN VMDetect::FileExists(std::string FileName)
{
	std::fstream FileToTest(FileName);
	if (FileToTest.is_open())
	{
		FileToTest.close();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOLEAN VMDetect::CheckDebuggerPresent(VOID)
{
	return IsDebuggerPresent(); // wraps so we get clean output.
}

// https://github.com/a0rtega/pafish/blob/master/pafish/vbox.c#L118
BOOLEAN VMDetect::VirtualBox::CheckSystemFiles(VOID)
{
	std::vector<std::string> Files2Check = {
		"C:\\WINDOWS\\system32\\vboxdisp.dll",
		"C:\\WINDOWS\\system32\\vboxhook.dll",
		"C:\\WINDOWS\\system32\\vboxmrxnp.dll",
		"C:\\WINDOWS\\system32\\vboxogl.dll",
		"C:\\WINDOWS\\system32\\vboxoglarrayspu.dll",
		"C:\\WINDOWS\\system32\\vboxoglcrutil.dll",
		"C:\\WINDOWS\\system32\\vboxoglerrorspu.dll",
		"C:\\WINDOWS\\system32\\vboxoglfeedbackspu.dll",
		"C:\\WINDOWS\\system32\\vboxoglpackspu.dll",
		"C:\\WINDOWS\\system32\\vboxoglpassthroughspu.dll",
		"C:\\WINDOWS\\system32\\vboxservice.exe",
		"C:\\WINDOWS\\system32\\vboxtray.exe",
		"C:\\WINDOWS\\system32\\VBoxControl.exe",
		"C:\\program files\\oracle\\virtualbox guest additions\\"
	};

	for (auto File : Files2Check)
	{
		if (FileExists(File))
		{
			std::cout << "!! Found: " << File << std::endl;
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN VMDetect::VirtualBox::CheckTrayWindowExists(VOID)
{
	HWND VBoxTrayToolClass = FindWindow(L"VBoxTrayToolWndClass", NULL);
	HWND VBoxTrayToolWnd = FindWindow(NULL, L"VBoxTrayToolWnd");

	return (VBoxTrayToolClass || VBoxTrayToolWnd);
}
	
BOOLEAN VMDetect::CPU::CheckCpuVendor(VOID)
{
	std::vector<std::string> DisallowedNames = {
		"KVMKVMKVM\0\0\0",
		"Microsoft Hv",
		"VMwareVMware",
		"XenVMMXenVMM",
		"prl hyperv  ",
		"VBoxVBoxVBox"
	};

	auto CurrentCPUName = InstructionSet::Vendor();

	for (auto Vender : DisallowedNames)
	{
		if (Vender == CurrentCPUName)
		{
			std::cout << "!! Vendor name matches disallowed \"" << Vender << "\"" << std::endl;
			return TRUE;
		}
	}

	return FALSE;
}

// https://github.com/a0rtega/pafish/blob/184b3fc3d5431e7334485a1eb33f30d3be125dc3/pafish/bochs.c#L18
BOOLEAN VMDetect::CPU::CheckBochsAMD(VOID)
{
	return (!memcmp(InstructionSet::Brand().data(), "AMD Athlon(tm) processor", 24)) ? TRUE : FALSE;
}

BOOLEAN VMDetect::CPU::CheckHVBit(VOID)
{
	int* values = new int[4];
	__cpuid(values, 0x01);
	return (values[2] >> 32) & 0x1;
}