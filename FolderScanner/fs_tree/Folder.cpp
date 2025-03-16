#include "Folder.h"
#include <iostream>
#include <algorithm>

namespace fs_tree
{
    void Folder::AddFolder(std::unique_ptr<Folder> folder)
    {
        std::unique_lock lock(mutex_);
        folders_.push_back(std::move(folder));
    }

    void Folder::AddFile(std::unique_ptr<File> file)
    {
        std::unique_lock lock(mutex_);
        files_.push_back(std::move(file));
    }



    const std::uint64_t Folder::GetFolderNum() const
    {
        return folders_.size();
    }

    Folders& Folder::GetFolders()
    {
        return folders_;
    }

    Files& Folder::GetFiles()
    {
        return files_;
    }

    std::optional<std::filesystem::directory_iterator> Folder::GetDirectoryIterator()
    {
        std::error_code ec;
        std::unique_lock lock(this->mutex_);
        std::filesystem::directory_iterator iterator(path_, ec);
		lock.unlock();

        if (ec) return std::nullopt;
        return iterator;
    }

    void Folder::CalculateSize()
    {
        for (auto& folder : folders_)
        {
            size_ += folder->size_;
        }

        for (auto& file : files_)
        {
            size_ += file->size_;
        }

        std::sort(files_.begin(), files_.end(), [](const auto& lhs, const auto& rhs) { return lhs->size_ > rhs->size_; });
        std::sort(folders_.begin(), folders_.end(), [](const auto& lhs, const auto& rhs) { return lhs->size_ > rhs->size_; });
    }

    std::uintmax_t Folder::RecursiveCalculateSize()
    {
        std::unique_lock lock(mutex_);

        for (auto& folder : folders_)
        {
            size_ += folder->RecursiveCalculateSize();
        }

        for (auto& file : files_)
        {
            size_ += file->size_;
        }

        std::sort(files_.begin(), files_.end(), [](const auto& lhs, const auto& rhs) { return lhs->size_ > rhs->size_; });
        std::sort(folders_.begin(), folders_.end(), [](const auto& lhs, const auto& rhs) { return lhs->size_ > rhs->size_; });

        return size_;
    }
    Folder::~Folder()
    {
#if _DEBUG
		std::cout << "Folder destructor called for: " << path_ << std::endl;
#endif
    }
    std::uintmax_t Folder::Size() const
    {
        return size_;
    }
}