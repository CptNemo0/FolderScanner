#ifndef FS_TREE_FILE
#define FS_TREE_FILE

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>
#include <unordered_map>

namespace fs_tree
{
	struct display_info
	{
		double size;
		std::string unit;
		std::string path;
		display_info(const std::filesystem::path& p, const std::uintmax_t sz);

		display_info() = default;
		display_info(const display_info&) = delete;
		display_info& operator=(const display_info&) = delete;

		display_info(display_info&& other) noexcept : size(other.size), unit(std::move(other.unit)), path(std::move(other.path)) {};
		display_info& operator=(display_info&& other) noexcept;

	};
	//double display_size(const std::uintmax_t size);

	class File;

	using Files = std::vector<std::unique_ptr<File>>;

	class File
	{
	public:
		const std::filesystem::path path_;
		const std::uintmax_t size_;
		File(const std::filesystem::path& path) : path_(path), size_(std::filesystem::file_size(path)) {};
		display_info GetDisplayInfo() const
		{
			return display_info(path_, size_);
		}
		virtual ~File();
	};
}

#endif // !FS_TREE_FILE