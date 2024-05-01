#pragma once

#include <fstream>
#include "BitIO.h"
#include "Model.h"

inline void initializeArithmeticDecoder(std::unique_ptr<stl::BitFile>& input, USHORT& code) {
	for (int i{ 0 }; i < 16; ++i) {
		code <<= 1;
		code |= stl::inputBit(input);
	}
}

inline long getCurrentIndex(Symbol& s, USHORT low, USHORT high, USHORT code) {
	long range{ high - low + 1 };
	long index = (long)(((code - low) + 1) * s.scale - 1) / range;
	return index;
}

void removeSymbolFromStream(std::unique_ptr<stl::BitFile>& input, Symbol& s, USHORT& low, USHORT& high, USHORT& code) {
	long range{ (high - low) + 1 };
	high = low + (USHORT)((range * s.highCount) / s.scale - 1);
	low = low + (USHORT)((range * s.lowCount) / s.scale);
	for (;;) {
		if ((high & 0x8000) == (low & 0x8000)) {
			//do nothing
		}
		else if ((low & 0x4000) && !(high & 0x4000)) {
			code ^= 0x4000;
			high |= (1 << 14);//set bit
			low &= ~(1 << 14);//clear bit
		}
		else
			return;
		low <<= 1;
		high <<= 1;
		high |= 1;
		code <<= 1;
		code |= stl::inputBit(input);
	}
}


std::string initialize(std::unique_ptr<stl::BitFile>& input, uint32_t& order) {
	std::string fileType;
	size_t i = 0;
	char c;
	input->file >> order;
	while (true) {
		input->file.get(c);
		if (c == 0) break;
		fileType.push_back(c);
		if (++i == 16) {
			printf("ERROR: File format not recognized\n");
			exit(1);
		}
	}
	return fileType;
}

void expandFile(std::unique_ptr<stl::BitFile>& input, std::fstream& output, uint32_t order, size_t fileSize) {
	Symbol s;
	int c{};
	USHORT low{ 0 }, high{ 0xffff }, code{ 0 };
	long index{ 0 };
	initializeModel(order);
	initializeArithmeticDecoder(input, code);
	uint64_t counter{ 0 };
	size_t sizeInKB{ 0 };
	fileSize = std::ceil(fileSize / float(1024));
	for (;;) {
		do {
			getSymbolScale(s);
			index = getCurrentIndex(s, low, high, code);
			c = convertSymbolToInt(index, s);
			removeSymbolFromStream(input, s, low, high, code);
		} while (c == ESCAPE);
		if (c == END_OF_STREAM)
			break;
		output.put(c);
		++counter;

		if (counter == 10240) {
			sizeInKB += 10;
			std::cout << sizeInKB << "KB/" << fileSize << "KB\r";
			counter = 0;
		}
		updateModel(c);
	}
	std::cout << std::ceil(sizeInKB + (counter / float(1024))) << "KB/" << fileSize << "KB\n";
}