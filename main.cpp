#include "VMDetect.hpp"
#include "InstructionSet_MS.hpp"

#define COLOUR_OK 10
#define COLOUR_BAD 12
#define COLOUR_NORMAL 7

// Only supporting Windows currently.
#ifdef _WIN64
#define PLATFORM "Windows 64 Bit"
#else
#define PLATFORM "Windows 32 Bit"
#endif

const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;

HANDLE hConsole;

BOOLEAN TestFunction(std::string Name, BOOLEAN(*Function)(VOID), std::size_t iDesiredSize = 50)
{ 
	if (!hConsole)
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	
	if (!hConsole)
	{
		exit(-1);
		return FALSE;
	}

	SetConsoleTextAttribute(hConsole, COLOUR_NORMAL);

	std::string FinalName = Name;
	if (Name.size() < iDesiredSize)
	{
		for (int i = Name.size(); i < iDesiredSize; i++)
		{
			FinalName.push_back(' ');
		}
	}

	printf("%s ", FinalName.c_str());
	
	auto bReturn = Function();
	//printf("%s\n", (bReturn ? "BAD" : "OK"));

	if (bReturn)
	{
		SetConsoleTextAttribute(hConsole, COLOUR_BAD);
		printf("VM DETECTED\n");
		SetConsoleTextAttribute(hConsole, COLOUR_NORMAL);
	}
	else 
	{
		SetConsoleTextAttribute(hConsole, COLOUR_OK);
		printf("BAD\n");
		SetConsoleTextAttribute(hConsole, COLOUR_NORMAL);
	}

	return bReturn;
}

#define Test(Func) TestFunction(#Func##".. ", Func)

std::string DashesGen(int n)
{
	std::string Out;
	for (int i = 0; i < n; i++)
	{
		Out.push_back('-');
	}

	return Out;
}

int main()
{
	std::cout << "Compact VM Detector || Compiled at " __TIME__ " " __DATE__ " for " PLATFORM "\n" << std::endl;
	std::string TitleString = std::string(InstructionSet::Vendor()) + std::string(" | ") + std::string(InstructionSet::Brand());
	std::cout << TitleString << std::endl;
	std::cout << DashesGen(TitleString.size()) << std::endl;

	Test(VMDetect::CheckDebuggerPresent);
	Test(VMDetect::CheckSyntheticMSRs);
	Test(VMDetect::CheckCPUIDLeafResponse);
	Test(VMDetect::VirtualBox::CheckSystemFiles);
	Test(VMDetect::VirtualBox::CheckTrayWindowExists);
	Test(VMDetect::CPU::CheckCpuVendor);
	Test(VMDetect::CPU::CheckHVBit);
	Test(VMDetect::CheckRTDSCTimings);
}