#ifndef FS_TREE_FOLDER
#define FS_TREE_FOLDER

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <memory>
#include <optional>
#include <vector>

#include "File.h"

namespace fs_tree
{
	class Folder;

	using Folders = std::vector<std::unique_ptr<Folder>>;
	
	class Folder
	{
	private:
		Files files_;
		Folders folders_;
		std::uintmax_t size_ = 0;

		std::mutex mutex_;

	public:
		const std::filesystem::path path_;
		Folder(const std::filesystem::path& path) : path_(path) {}
		void AddFolder(std::unique_ptr<Folder> folder);
		void AddFile(std::unique_ptr<File> file);

		const std::uint64_t GetFolderNum() const;
		Folders& GetFolders();
		Files& GetFiles();
		std::optional<std::filesystem::directory_iterator> GetDirectoryIterator();
		void CalculateSize();
        std::uintmax_t RecursiveCalculateSize();
		virtual ~Folder();
		std::uintmax_t Size() const;

		display_info GetDisplayInfo() const
		{
			return display_info(path_, size_);
		}
	};
}

#endif // FS_TREE_FOLDER



