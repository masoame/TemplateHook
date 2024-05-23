#include"common.hpp"
namespace AnalyzePE
{
	template<typename CHART>
	inline std::unique_ptr<char[]> LoadFile(const CHART* filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open() == false)return nullptr;

		std::streampos filesize = file.tellg();

		file.seekg(std::ios::beg);
		std::unique_ptr<char[]> buf{ new char[filesize] };
		file.read(buf.get(), filesize);
		file.close();

		return buf;
	}

	extern void PrintPE_Message(LPVOID pFileBuffer);
	extern size_t RVAtoFOA(LPVOID pFileBuffer, size_t RVA);
	extern size_t FOAtoRVA(LPVOID pFileBuffer, size_t FOA);
}