#include "Analyzer.h"
#include "../fs_tree/File.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <syncstream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <windows.h>
#include <atomic>

namespace anal
{
	namespace
	{
        std::queue<fs_tree::Folder*> work_queue_;
        std::mutex work_queue_mutex_;

        std::condition_variable loading_condition_variable_;
        
        std::atomic_int accessed_ = 0;
        std::atomic_int loading_working_ = 0;
        std::atomic_bool loading_finished_ = true;
        std::atomic_bool calculating_finished_ = true;

        void PushWork(fs_tree::Folder* folder)
        {
            std::unique_lock lock(work_queue_mutex_);
            work_queue_.push(folder);
            loading_condition_variable_.notify_one();
        }   

        fs_tree::Folder* PopWork()
        {
            std::unique_lock lock(work_queue_mutex_);
            if (work_queue_.empty()) return nullptr;
            auto top = work_queue_.front();
            work_queue_.pop();
            return top;
        }

        fs_tree::Folder* PopWorkNoLock()
        {
            if (work_queue_.empty()) return nullptr;
            auto top = work_queue_.front();
            work_queue_.pop();
            return top;
        }

        std::uint64_t QueueSize()
        {
            std::unique_lock lock(work_queue_mutex_);
            return work_queue_.size();
        }

        void LoadFolder(fs_tree::Folder* folder)
        {
            loading_working_.fetch_add(1);
            if (!folder) return;

            try
            {
                const auto iterator = folder->GetDirectoryIterator();
                if (iterator != std::nullopt)
                {
                    auto analyze_lambda = [&](const std::filesystem::directory_entry& item)
                        {
                            accessed_.fetch_add(1);
                            const auto status = item.status();
                            const auto type = status.type();
                            const auto path = item.path();

                            if (std::filesystem::exists(path))
                            {
                                switch (type)
                                {
                                case std::filesystem::file_type::regular:
                                {
                                    auto file = std::make_unique<fs_tree::File>(path);
                                    folder->AddFile(std::move(file));
                                    break;
                                }
                                case std::filesystem::file_type::directory:
                                {
                                    auto directory = std::make_unique<fs_tree::Folder>(path);
                                    PushWork(directory.get());
                                    folder->AddFolder(std::move(directory));
                                    break;
                                }
                                }
                            }
                        };

                    std::ranges::for_each(iterator.value(), analyze_lambda);
                }
            }
            catch (std::exception e)
            {
                std::osyncstream(std::cout) << e.what() << " " << folder->path_ << std::endl;
            }


            loading_working_.fetch_sub(1);
        }

        void LoadFolderThread(std::uint32_t internal_tid)
        {
            const auto mask = 1 << internal_tid;
            SetThreadAffinityMask(GetCurrentThread(), mask);
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
            auto id = std::this_thread::get_id()._Get_underlying_id();
            while (true)
            {
                fs_tree::Folder* folder = nullptr;
                {
                    std::unique_lock lock(work_queue_mutex_);

                    loading_condition_variable_.wait(lock,
                        [&]()
                        {
                            return !work_queue_.empty() || loading_finished_.load();
                        });

                    if (loading_finished_.load()) break;

                    folder = PopWorkNoLock();
                }

                LoadFolder(folder);
                if (loading_finished_.load())  break;
            }
#if _DEBUG
			std::osyncstream(std::cout) << "Loader thread: " << id << " exits.\n";
#endif 
        }

        void LoadFolderManagerThread(fs_tree::FilesystemTree* filesystem_tree)
        {
            auto previous_working = 0;
            auto previous_accessed = 0;
            std::uint32_t counter = 0;
            while (true)
            {
                const auto w = loading_working_.load();
                const auto a = accessed_.load();

                if (w != previous_working || a != previous_accessed)
                {
                    previous_working = w;
                    previous_accessed = a;
                    counter = 0;
                }
                else if (a == previous_accessed && w == previous_working)
                {
                    counter++;
                }

                if (counter > 10)
                {
                    break;
                }

                if (loading_finished_.load())
                {
#if _DEBUG
                    std::osyncstream(std::cout) << "Manager Thread exits.\n";
#endif
                    return;
                }

                Sleep(100);
            }

            Sleep(400);

            std::osyncstream(std::cout) << "Loading folders and files concluded! \n";
          
            FinishLoadingFolders();

            std::osyncstream(std::cout) << "Calculating sizes... "<< "\n";

			std::vector<std::thread> calculator_threads;
            const auto max_thread_num = std::thread::hardware_concurrency();

            auto calculate_lambda = [](fs_tree::Folder* folder, std::uint32_t internal_tid)
            {
                const auto mask = 1 << internal_tid;
                SetThreadAffinityMask(GetCurrentThread(), mask);
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
                auto id = std::this_thread::get_id()._Get_underlying_id();

                folder->RecursiveCalculateSize();
#if _DEBUG
                std::osyncstream(std::cout) << "Calculator thread: " << id << " exits.\n";
#endif  
            };

            auto root = filesystem_tree->GetRoot();            
            auto& subfolders = root->GetFolders();
            
            std::uint32_t tid_ctr = 0;
            for (auto& folder : subfolders)
            {
				calculator_threads.push_back(std::thread(calculate_lambda, folder.get(), tid_ctr % max_thread_num));
				tid_ctr++;
            }

			for (auto& thread : calculator_threads)
			{
				thread.join();
			}

			root->CalculateSize();
			calculating_finished_.store(true);

            std::osyncstream(std::cout) << "Calculating concluded! \nAccessed: " << accessed_.load() << " files and folders. \nType 'ls' and press 'enter' to print results.\n";

#if _DEBUG
            std::osyncstream(std::cout) << "Manager Thread exits.\n";
#endif
        }
	}
	
    void AnalyzeFilesystemTree(fs_tree::FilesystemTree* filesystem_tree)
    {
		while(work_queue_.size()) work_queue_.pop();
		accessed_.store(0);
		loading_working_.store(0);
		loading_finished_.store(false);
        calculating_finished_.store(false);
        PushWork(filesystem_tree->GetRoot());
        
        const auto max_thread_num = std::thread::hardware_concurrency();

        for (std::uint32_t i = 0; i < max_thread_num; i++)
        {
            std::thread loader_thread(LoadFolderThread, i);
            loader_thread.detach();
        }

        std::thread loader_thread_manager(LoadFolderManagerThread, filesystem_tree);
        loader_thread_manager.detach();
    }

    void FinishLoadingFolders()
    {
        loading_finished_.store(true);
        loading_condition_variable_.notify_all();

    }

    bool LoadingFinished()
    {
        return loading_finished_.load();
    }

    bool ProcessingFinished()
    {
        return calculating_finished_.load() && loading_finished_.load();
    }

   

    std::condition_variable& GetLoadingConditionVariable()
    {
		return loading_condition_variable_;
    }
}