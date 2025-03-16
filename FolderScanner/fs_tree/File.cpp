#include "File.h"

#include <iostream>

namespace fs_tree
{
	namespace
	{
		const std::unordered_map<int, std::string> size_units{
			{0, "B"},
			{1, "KB"},
			{2, "MB"},
			{3, "GB"},
			{4, "TB"},
			{5, "PB"},
			{6, "EB"},
			{7, "ZB"},
			{8, "YB"}
		};
	}

	display_info::display_info(const std::filesystem::path& p, const std::uintmax_t sz)
	{
		// Use filename if available, otherwise use the last part of parent path (for folder paths ending with a separator).
		auto last = p.filename();
		if (last.empty())
		{
			last = p.parent_path().filename();
		}
		path = last.string();

		int depth = 0;
		size = static_cast<double>(sz);

		while (size > 1024.0)
		{
			size /= 1024.0;
			depth++;
		}
		//size = ceil(size);
		unit = size_units.at(depth);
	}

	display_info& display_info::operator=(display_info&& other) noexcept
	{
		if (this != &other)
		{
			size = other.size;
			unit = std::move(other.unit);
			path = std::move(other.path);
		}
		return *this;
	}

	File::~File()
	{
#if _DEBUG
		std::cout << "File destructor called for: " << path_ << std::endl;
#endif
	}
}