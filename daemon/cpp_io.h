#pragma once

#include <string>
#include <memory>
#include <array>


namespace tcpserv {

	class Printer;

	using SPrinter = std::shared_ptr<Printer>;

	class Printer {
	public:

		static std::string getCpuStats();

		static std::string getCpuStatsStaged(const std::array<int, 10> &prevMap);

		static std::string getMemStats();

		static std::array<int, 10> getCpuValues();
	};
}
