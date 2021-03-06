#include "SettingsParser.h"

#include "settings_utils.h"

#include <boost/any.hpp>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <algorithm>

namespace carl::settings {

std::ostream& operator<<(std::ostream& os, const boost::any& val) {
	if (val.empty()) {
		return os << "<empty>";
	} else if (boost::any_cast<bool>(&val) != nullptr) {
		return os << std::boolalpha << boost::any_cast<bool>(val);
	} else if (boost::any_cast<std::size_t>(&val) != nullptr) {
		return os << boost::any_cast<std::size_t>(val);
	} else if (boost::any_cast<std::string>(&val) != nullptr) {
		return os << boost::any_cast<std::string>(val);
	} else if (boost::any_cast<carl::settings::duration>(&val) != nullptr) {
		return os << boost::any_cast<carl::settings::duration>(val);
	} else if (boost::any_cast<carl::settings::binary_quantity>(&val) != nullptr) {
		return os << boost::any_cast<carl::settings::binary_quantity>(val);
	} else if (boost::any_cast<carl::settings::metric_quantity>(&val) != nullptr) {
		return os << boost::any_cast<carl::settings::metric_quantity>(val);
	}
	return os << "Unknown type";
}


std::ostream& operator<<(std::ostream& os, OptionPrinter op) {
	if (op.parser.argv_zero == nullptr) {
		os << "Usage: <binary> [options]";
	} else {
		os << "Usage: " << op.parser.argv_zero << " [options]";
	}
	for (unsigned i = 0; i < op.parser.mPositional.max_total_count(); ++i) {
		os << " " << op.parser.mPositional.name_for_position(i);
	}
	os << std::endl;
	return os << op.parser.mAllOptions;
}
std::ostream& operator<<(std::ostream& os, SettingsPrinter sp) {
	for (const auto& option: sp.parser.mAllOptions.options()) {
		auto value = sp.parser.mValues[option->canonical_display_name()];
		os << option->canonical_display_name() << " = " << value.value() << std::endl;
	}
	return os;
}

void SettingsParser::warn_for_unrecognized(const po::parsed_options& parsed) const {
	auto res = po::collect_unrecognized(parsed.options, po::exclude_positional);
	for (const auto& s: res) {
		warn_for_unrecognized_option(s);
	}
}

void SettingsParser::parse_command_line(int argc, char* argv[], bool allow_unregistered) {
	auto parser = po::command_line_parser(argc, argv).options(mAllOptions).positional(mPositional);
	if (allow_unregistered) {
		parser.allow_unregistered();
	}
	auto parsed = parser.run();
	if (allow_unregistered) {
		warn_for_unrecognized(parsed);
	}
	po::store(parsed, mValues);
}

void SettingsParser::parse_config_file(bool allow_unregistered) {
	fs::path configfile(mValues[this->name_of_config_file()].as<std::string>());
	if (fs::is_regular_file(configfile)) {
		auto parsed = po::parse_config_file<char>(configfile.c_str(), mAllOptions, allow_unregistered);
		if (allow_unregistered) {
			warn_for_unrecognized(parsed);
		}
		po::store(parsed, mValues);
	} else {
		warn_config_file(configfile);
	}
}

bool SettingsParser::finalize_settings() {
	return std::any_of(mFinalizer.begin(), mFinalizer.end(),
		[](const auto& f){ return f(); }
	);
}

void SettingsParser::finalize() {
	for (const auto& po: mOptions) {
		mAllOptions.add(po);
	}
}

void SettingsParser::parse_options(int argc, char* argv[], bool allow_unregistered) {
	argv_zero = argv[0];
	parse_command_line(argc, argv, allow_unregistered);
	if (mValues.count(this->name_of_config_file()) != 0) {
		parse_config_file(allow_unregistered);
	}
	po::notify(mValues);
	if (finalize_settings()) {
		po::notify(mValues);
	}
}

}