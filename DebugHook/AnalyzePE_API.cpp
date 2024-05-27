#include "AnalyzePE_API.hpp"
#include "time.h"

void AnalyzePE::PrintPE_Message(LPVOID pFileBuffer)
{
	std::stringstream ss;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (pDosHeader->e_magic != 0x5A4D) return;

	ss << "Ntͷƫ��: " << pDosHeader->e_lfanew << std::endl << std::endl;;

	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD64)pFileBuffer + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != 0x4550) return;

	PIMAGE_FILE_HEADER pFileHeader = &pNtHeader->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = &pNtHeader->OptionalHeader;
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);

	ss << "\n�ļ�ͷƫ�� " << (DWORD64)pFileHeader - (DWORD64)pFileBuffer << std::endl << std::endl;

	ss << "--\tcpu����: " << pFileHeader->Machine << std::endl;
	ss << "--\t�ļ�����: " << pFileHeader->Characteristics << std::endl;
	ss << "--\t������: " << pFileHeader->NumberOfSections << std::endl;
	ss << "--\t���ű��з��ŵĸ���: " << pFileHeader->NumberOfSymbols << std::endl;
	ss << "--\tʱ���: " << pFileHeader->TimeDateStamp << std::endl;
	ss << "--\t��ѡͷ��С: " << pFileHeader->SizeOfOptionalHeader << std::endl;
	ss << "--\tָ����ű�: " << pFileHeader->PointerToSymbolTable << std::endl;

	ss << "\n��ѡͷƫ�� " << (DWORD64)pOptionHeader - (DWORD64)pFileBuffer << std::endl << std::endl;;
	ss << "\n������ƫ��" << (DWORD64)pSectionHeader - (DWORD64)pFileBuffer << std::endl << std::endl;;

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

size_t AnalyzePE::FOAtoRVA(LPVOID pFileBuffer, size_t FOA)
{
	return size_t();
}