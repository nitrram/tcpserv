#include "cpp_io.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <iostream>

namespace tcpserv {

	std::string Printer::getMemStats() {

		std::ifstream fs("/proc/meminfo");

		std::string lineElem;
		std::string line;
		std::array<int, 5> map;
		std::array<int, 5>::size_type i = 0;
		while(std::getline(fs, line)) {
			std::istringstream iss(line);
			while(std::getline(iss, lineElem, ' ')) {
				std::cout << "non-filter " << lineElem << std::endl;
				if(lineElem.empty() || (lineElem.find(":") != std::string::npos) || lineElem == "kB")
					continue;
				std::cout << lineElem << std::endl;
				map[i] = std::atoi(lineElem.c_str());
				break;
			}
			// we need just 5 lines to compute the memory
			if(++i >= 5)
				break;
		}

		std::ostringstream oss;

		oss.precision(2);

		oss << (static_cast<float>(map[0]-map[1]-map[3]-map[4]) / 1024.f);

		return oss.str();
	}


	std::string Printer::getCpuStats() {
		std::array<int, 10> map, prev_map;

		prev_map = getCpuValues();

		std::this_thread::sleep_for(std::chrono::milliseconds(400));

		map = getCpuValues();

		int prev_idle = prev_map[3] + prev_map[4];
		int idle = map[3] + map[4];

		int prev_non_idle = prev_map[0] + prev_map[1] + prev_map[2] + prev_map[5] + prev_map[6] + prev_map[7];
		int non_idle = map[0] + map[1] + map[2] + map[5] + map[6] + map[7];

		int totald = (idle + non_idle) - (prev_idle + prev_non_idle);
		int idled = (idle - prev_idle);

		std::ostringstream oss;

		oss.precision(8);

		oss << static_cast<float>(totald - idled) / static_cast<float>(totald * 100.f) << " [%]\n";

		return oss.str();
	}

	std::array<int, 10> Printer::getCpuValues() {

		std::array<int, 10> out;

		std::ifstream fs("/proc/stat");

		std::string lineElem;
		std::array<int, 10>::size_type i = 0;
		while(std::getline(fs, lineElem, ' ')) {

			if(lineElem.empty() || lineElem == "cpu")
				continue;

			out[i++] = std::stoi(lineElem);
		}

		return out;
	}
}
