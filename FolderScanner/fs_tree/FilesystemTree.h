#ifndef FILESYSTEM_FILESYSTEM_TREE_H
#define FILESYSTEM_FILESYSTEM_TREE_H

#include "Folder.h"
#include "File.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>

namespace fs_tree
{
	class FilesystemTree
	{
	private:
		std::unique_ptr<Folder> root_;

	public:
		FilesystemTree(const std::filesystem::path& root_path) : root_(std::move(std::make_unique<Folder>(root_path))) 
		{
		
		};
		Folder* GetRoot() const;

		void AddFile(File* file);
		void AddFolder(Folder* file);
		std::optional<Folder*> GetFolder(std::string_view path);
		std::optional<File*> GetFile(std::string_view path);
	};
}

#endif // !FILESYSTEM_FILESYSTEM_TREE_H