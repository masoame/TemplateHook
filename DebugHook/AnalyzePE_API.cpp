#include "AnalyzePE_API.hpp"

void AnalyzePE::PrintPE_Message(LPVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (pDosHeader->e_magic != 0x5A4D) return;

	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD64)pFileBuffer + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != 0x4550) return;

	PIMAGE_FILE_HEADER pFileHeader = &pNtHeader->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = &pNtHeader->OptionalHeader;

	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);
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
