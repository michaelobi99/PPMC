#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <chrono>
#include <format>
#include "Decoder.h"
#include "Encoder.h"

namespace fs = std::filesystem;


struct Timer {
public:
	Timer() = default;
	void Start() {
		start = std::chrono::high_resolution_clock::now();
	}
	void Stop() {
		stop = std::chrono::high_resolution_clock::now();
	}
	float time() {
		elapsedTime = std::chrono::duration<float>(stop - start).count();
		return elapsedTime;
	}
private:
	float elapsedTime{};
	std::chrono::time_point<std::chrono::high_resolution_clock> start, stop;
};


uintmax_t fileSize(fs::path const& path) {
	auto lengthInbytes = fs::file_size(path);
	return lengthInbytes;
}

void help() {
	printf("Usage: ppmc -[c, x] -[depth (0-16)] [filenames or folder]\n");
	exit(1);
}

void encode(uint32_t order, const fs::path& ifile, const fs::path& ofile = fs::path()) {
	auto timer = Timer();
	std::string filename = ifile.filename().string();
	std::string inputFile = (ifile.parent_path() /= ifile.filename()).string();
	std::string compressedFile;
	std::string extension = ".ppmc";
	std::string fileType;
	if (size_t idx = filename.find_last_of('.'); idx == std::string::npos) {
		if (ofile.empty())
			compressedFile = (ifile.parent_path() /= fs::path{ filename + extension }).string();
		else
			compressedFile = (ofile.parent_path() /= ofile.filename() /= fs::path{ filename + extension }).string();
	}
	else {
		fileType = filename.substr(idx, std::size(filename) - idx);
		if (ofile.empty())
			compressedFile = (ifile.parent_path() /= fs::path{ filename.substr(0, idx) + extension }).string();
		else
			compressedFile = (ofile.parent_path() /= ofile.filename() /= fs::path{ filename.substr(0, idx) + extension }).string();
	}
	try {
		float avgEff{ 0.0 };
		std::fstream input(inputFile, std::ios_base::in | std::ios_base::binary);
		if (!input.is_open()) {
			std::cout << "Invalid file path!!\n";
			exit(1);
		}
		auto output = stl::OpenOutputBitFile(compressedFile);
		timer.Start();
		compressFile(input, output, order, fileType, fileSize(fs::path(inputFile)));
		timer.Stop();
		printf("%s encoding time = %f seconds\n", filename.c_str(), timer.time());
		input.close();
		stl::closeOutputBitFile(output);
		float efficiency = (8 * fileSize(fs::path(compressedFile)) / float(fileSize(fs::path(inputFile))));
		std::cout << std::format("compression efficiency = {} \n\n", efficiency);
	}
	catch (stl::FileError const& error) {
		std::cout << error.what();
	}
	catch (std::exception const& error) {
		std::cout << error.what();
	}
}


void decode(const fs::path& file) {
	auto timer = Timer();
	std::uint32_t order{0};
	std::string filename = file.filename().string();
	std::string inputFile = (file.parent_path() /= file.filename()).string();
	std::string originalFile;
	std::string fileType;
	std::string extension = ".ppmc";
	if (file.extension().string() != extension) {
		printf("Error: File not recognized\n");
		exit(1);
	}
	try {
		auto input = stl::OpenInputBitFile(inputFile);
		fileType = initialize(input, order);
		if (size_t idx = filename.find_last_of('.'); idx == std::string::npos) {
			originalFile = (file.parent_path() /= fs::path{ filename + fileType }).string();
		}
		else {
			originalFile = (file.parent_path() /= fs::path{ filename.substr(0, idx) + fileType }).string();
		}
		std::fstream output(originalFile, std::ios_base::out | std::ios_base::binary);
		timer.Start();
		expandFile(input, output, order, fileSize(fs::path(file)));
		timer.Stop();
		printf("PPM decoding time = %f seconds\n", timer.time());
		stl::closeInputBitFile(input);
		output.close();
	}
	catch (stl::FileError const& error) {
		std::cout << error.what()<<"\n";
	}
	catch (std::exception const& error) {
		std::cout << error.what();
	}
	
}

int main(int argc, char* argv[]) {
	uint32_t order{ 0 };
	int i{};
	if (argc == 1) help();

	if (strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-x") != 0) {
		printf("Error: commands not recognized\n");
		exit(1);
	}

	if (strcmp(argv[1], "-c") == 0) {
		if (argc < 4) {
			printf("Error: number of arguments invalid\n");
			exit(1);
		}
		if (char c; (sscanf(argv[2], "%c%d", &c, &order) == EOF) || c != '-' || order < 0 || order > 16) {
			printf("Error: invalid arguments\n");
			help();
		}
		i = 3;
		for (; i < argc; ++i) {
			auto file = fs::path(argv[i]);
			if (fs::is_regular_file(file)) {
				encode(order, file);
			}
			else if (fs::is_directory(file)) {
				fs::path compressedFolder = fs::path{ (file.parent_path() /= file.filename()).string() + ".ppmc" };
				bool success = fs::create_directory(compressedFolder);
				if (!success) {
					std::cerr << "Error creating folder\n";
					return 1;
				}
				for (fs::path f : fs::directory_iterator(file)) {
					encode(order, f, compressedFolder);
				}
			}
			else {
				printf("Error: file not found\n");
			}
		}
	}
	else {
		if (argc < 3) {
			printf("Error: number of arguments invalid\n");
			exit(1);
		}
		i = 2;
		for (; i < argc; ++i) {
			auto file = fs::path(argv[i]);
			if (fs::is_regular_file(file)) {
				decode(file);
			}
			else if (fs::is_directory(file)) {
				std::string directoryName = file.filename().string();
				if (!directoryName.ends_with(".ppmc")) {
					printf("Error: Compressed folder not recognized\n");
				}
				/*fs::path compressedFolder = fs::path{ (file.parent_path() /= file.filename()).string() + ".ppmc" };
				bool success = fs::create_directory(compressedFolder);
				if (!success) {
					std::cerr << "Error creating folder\n";
					return 1;
				}
				std::cout << "Got here\n";*/
				for (auto& f : fs::directory_iterator(file)) {
					decode(f);
				}
			}
			else {
				printf("Error: file not found\n");
			}
		}
	}
}