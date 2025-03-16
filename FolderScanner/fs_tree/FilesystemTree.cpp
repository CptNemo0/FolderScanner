#include "FilesystemTree.h"

namespace fs_tree
{
	Folder* FilesystemTree::GetRoot() const
	{
		return root_.get();
	}
	void FilesystemTree::AddFile(File* file)
	{

	}
	void FilesystemTree::AddFolder(Folder* file)
	{
	}
	std::optional<Folder*> FilesystemTree::GetFolder(std::string_view path)
	{
		return std::optional<Folder*>();
	}
	std::optional<File*> FilesystemTree::GetFile(std::string_view path)
	{
		return std::optional<File*>();
	}
}