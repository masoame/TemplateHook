#include "AnalyzePE_API.hpp"
#include "time.h"

void AnalyzePE::PrintPE_Message(LPVOID pFileBuffer)
{
	std::stringstream ss;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (pDosHeader->e_magic != 0x5A4D) return;

	ss << "Nt头偏移: " << pDosHeader->e_lfanew << std::endl << std::endl;;

	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD64)pFileBuffer + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != 0x4550) return;

	PIMAGE_FILE_HEADER pFileHeader = &pNtHeader->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = &pNtHeader->OptionalHeader;
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);

	ss << "\n文件头偏移 " << (DWORD64)pFileHeader - (DWORD64)pFileBuffer << std::endl << std::endl;

	ss << "--\tcpu类型: " << pFileHeader->Machine << std::endl;
	ss << "--\t文件属性: " << pFileHeader->Characteristics << std::endl;
	ss << "--\t节区数: " << pFileHeader->NumberOfSections << std::endl;
	ss << "--\t符号表中符号的个数: " << pFileHeader->NumberOfSymbols << std::endl;
	ss << "--\t时间戳: " << pFileHeader->TimeDateStamp << std::endl;
	ss << "--\t可选头大小: " << pFileHeader->SizeOfOptionalHeader << std::endl;
	ss << "--\t指向符号表: " << pFileHeader->PointerToSymbolTable << std::endl;

	ss << "\n可选头偏移 " << (DWORD64)pOptionHeader - (DWORD64)pFileBuffer << std::endl << std::endl;;
	ss << "\n节区表偏移" << (DWORD64)pSectionHeader - (DWORD64)pFileBuffer << std::endl << std::endl;;

	std::cout << ss.str();
}

size_t AnalyzePE::RVAtoFOA(LPVOID pFileBuffer, size_t RVA)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (pDosHeader->e_magic != 0x5A4D) return NULL;
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD64)pFileBuffer + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != 0x4550) return NULL;

	PIMAGE_FILE_HEADER pFileHeader = &pNtHeader->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = &pNtHeader->OptionalHeader;

	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);

	if (RVA < pSectionHeader->VirtualAddress)//判断RVA是否在PE头区
	{
		if (RVA < pSectionHeader->PointerToRawData)//此时RVA==FOA判断FOA会不会溢出
			return RVA;
		else
			return NULL;
	}
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)//循环遍历节表头
	{
		if (RVA >= pSectionHeader[i].VirtualAddress)//是否大于这个节表的RVA
		{
			if (RVA <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].SizeOfRawData)//判断是否在这个节区
				return (RVA - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;//确定节区后，计算FOA
		}
		else
			return NULL;
	}
	return NULL;
}

size_t AnalyzePE::FOAtoRVA(LPVOID pFileBuffer, size_t FOA)
{
	return size_t();
}