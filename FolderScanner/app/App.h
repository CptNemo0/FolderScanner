#ifndef APP_APP_H
#define APP_APP_H

#include <iostream>
#include <syncstream>
#include <unordered_map>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <memory>
#include "../fs_tree/FilesystemTree.h"


namespace app
{
	class App
	{
	private:

		std::unordered_map<std::string_view, std::pair<std::function<void(const std::vector<std::string>&)>, std::string_view>> commands_ = 
		{
			{ 
				"cls", 
				{
					[this](const std::vector<std::string>& args) { system("cls"); },
					" |Cleans screen.                                         | 0 arguments"
				}
			},
			{ 
				"exit", 
				{
					[this](const std::vector<std::string>& args) { Exit(); },        
					"|Exits the application.                                 | 0 arguments"
				}
			},
			{ 
				"help", 
				{
					[this](const std::vector<std::string>& args) { Help(); },        
					"|Prints all available commands with their descriptions. | 0 arguments"
				}
			},
			{ 
				"scan", 
				{
					[this](const std::vector<std::string>& args) { Scan(args); },    
					"|Scans the specified folder.                            | argument 1: path to a folder (don't use \"\")\n"
					"        |                                                       | if no arguments are passed - scans the current folder"
				}
			},
			{
				"ls",
				{
					[this](const std::vector<std::string>& args) { Ls(args); },
					"  |Prints the results of the scan.                        | argument 1: minimum size in bytes of displayed file\\folder\n"
					"        |                                                       | argument 2: maximum size in bytes of displayed file\\folder"
				}			
			},
			{
				"cd",
				{
					[this](const std::vector<std::string>& args) { Cd(args); },
					"  |Changes the current directory.                         | argument 1: path to a folder (don't use \"\")"
				}
			},
			{
				"rmdir",
				{
					[this](const std::vector<std::string>& args) { Rmdir(args); },
					"|Removes the specified folder.                          | argument 1: path to a folder (don't use \"\")"
				}
			}
		};

		bool app_finished_ = false;

		std::unique_ptr<fs_tree::FilesystemTree> filesystem_tree_;

		fs_tree::Folder* current_root = nullptr;

		std::filesystem::path current_path_;

		std::optional<std::pair<std::string, std::vector<std::string>>> GetCommandAndArgs();
		std::vector<std::string> ParseCommand(const std::string& command);
		void RunCommand(const std::string& command, const std::vector<std::string>& args);

		void Help();
		void Exit();
		void Scan(const std::vector<std::string>& args);
		void Ls(const std::vector<std::string>& args);
		void Cd(const std::vector<std::string>& args);
		void Rmdir(const std::vector<std::string>& args);
	public:
		App();
		App(wchar_t* path);
		~App();
		void Run();
	};
}

#endif // !APP_APP_H
