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

	if (RVA < pSectionHeader->VirtualAddress)//�ж�RVA�Ƿ���PEͷ��
	{
		if (RVA < pSectionHeader->PointerToRawData)//��ʱRVA==FOA�ж�FOA�᲻�����
			return RVA;
		else
			return NULL;
	}
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)//ѭ�������ڱ�ͷ
	{
		if (RVA >= pSectionHeader[i].VirtualAddress)//�Ƿ��������ڱ��RVA
		{
			if (RVA <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].SizeOfRawData)//�ж��Ƿ����������
				return (RVA - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;//ȷ�������󣬼���FOA
		}
		else
			return NULL;
	}
	return NULL;
}
