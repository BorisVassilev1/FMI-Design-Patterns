#include <reportData.hpp>

void compare(const ReportData &lhs, const ReportData &rhs, std::ostream &os) {
	const char *OK		 = "OK       ";
	const char *DELETED	 = "DELETED  ";
	const char *NEW		 = "NEW      ";
	const char *MODIFIED = "MODIFIED ";

	auto l = lhs.begin();
	auto r = rhs.begin();
	while (l != lhs.end() && r != rhs.end()) {
		if (l->path < r->path) {
			os << DELETED << l->path << std::endl;
			++l;
		} else if (r->path < l->path) {
			os << NEW << r->path << std::endl;
			++r;
		} else {
			if (l->checksum != r->checksum) { os << MODIFIED << l->path << std::endl; }
			else { os << OK << l->path << std::endl; }
			++l;
			++r;
		}
	}
	while (l != lhs.end()) {
		os << DELETED << l->path << std::endl;
		++l;
	}
	while (r != rhs.end()) {
		os << NEW << r->path << std::endl;
		++r;
	}
}
