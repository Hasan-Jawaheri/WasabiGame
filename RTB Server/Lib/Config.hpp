#pragma once

#include <string>
#include <unordered_map>

namespace HBUtils {

	class Config {
		std::unordered_map<std::string, void*> m_configTable;

	public:
		Config() {
			m_configTable = {
				{"tcpPort", (void*)9965},
				{"udpPort", (void*)9966},
				{"hostname", (void*)"0.0.0.0"},
				{"clientBufferSize", (void*)4096},
			};
		}

		template<typename T>
		T Get(std::string propertyName) {
			return (T)m_configTable[propertyName];
		}

		template<>
		std::string Get<std::string>(std::string propertyName) {
			return std::string((char*)m_configTable[propertyName]);
		}
	};

};
