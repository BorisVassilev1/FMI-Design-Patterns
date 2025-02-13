#pragma once

#include <FSTree.hpp>
#include <nlohmann/json.hpp>
#include <factory.hpp>
#include <utils.hpp>

class FileData {
   public:
	std::filesystem::path path;
	std::string			  checksum;
};

using ReportData = std::vector<FileData>;

class ReportDataBuilder {
   public:
	virtual ReportData build(std::istream &) = 0;
	virtual ~ReportDataBuilder()			 = default;
};

class GNUReportDataBuilder : public ReportDataBuilder {
   public:
	ReportData build(std::istream &is) override {
		ReportData	data;
		std::string line;
		while (std::getline(is, line)) {
			std::istringstream ss(line);
			std::string		   checksum;
			std::string		   path;
			ss >> checksum;
			while (std::isspace(ss.peek()) || ss.peek() == '*')
				ss.ignore();
			ss >> path;
			data.push_back(FileData(path, checksum));
		}
		return data;
	}
};

class JSONReportDataBuilder : public ReportDataBuilder {
   public:
	ReportData build(std::istream &is) override {
		ReportData	   data;
		nlohmann::json j;
		is >> j;
		for (const auto &item : j) {
			data.push_back(FileData(item["path"], item["checksum"]));
		}
		return data;
	}
};

using ReportDataBuilderFactory = Factory<ReportDataBuilder>;

JOB(report_data_builder_factory_register, {
	ReportDataBuilderFactory::instance().registerType<GNUReportDataBuilder>("gnu");
	ReportDataBuilderFactory::instance().registerType<JSONReportDataBuilder>("json");
});

void compare(const ReportData &lhs, const ReportData &rhs, std::ostream &os);
