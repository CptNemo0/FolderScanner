#include "App.h"
#include "../analyzer/Analyzer.h"
#include <thread>
#include <sstream>
std::optional<std::pair<std::string, std::vector<std::string>>> app::App::GetCommandAndArgs()
{
	std::string command;
	std::cout << current_path_.string() <<" >> ";
	std::getline(std::cin, command);

	if (command.empty()) return std::nullopt;

	auto command_and_args = ParseCommand(command);
	const auto search = commands_.find(command_and_args[0]);

	if (search == commands_.end())
	{
		std::cout << " Command not found!" << std::endl;
		return std::nullopt;
	}
	
	command = command_and_args[0];
	std::vector<std::string> args(command_and_args.size());
	std::move(command_and_args.begin() + 1, command_and_args.end(), args.begin());

	auto return_value = std::make_optional<std::pair<std::string, std::vector<std::string>>>();
	return_value.value().first = std::move(command);
	return_value.value().second = std::move(args);
	return return_value;
}

void app::App::Help()
{
	std::cout << "----------------------------------------------------------------------------------------------------------------------------" << std::endl;
	for (const auto& [command, commandInfo] : commands_)
	{
		std::cout << command << "    " << commandInfo.second << std::endl;
		std::cout << "----------------------------------------------------------------------------------------------------------------------------" << std::endl;
	}
}

void app::App::Exit()
{
	app_finished_ = true;
}

void app::App::Scan(const std::vector<std::string>& args)
{
	std::cout << "Processing. Wait for the results..." << std::endl;
	
	std::filesystem::path path;

	if (args[0].size())
	{
		path = args[0];
	}
	else
	{
		path = current_path_;
	}

	std::cout << path << std::endl;

	filesystem_tree_ = std::make_unique<fs_tree::FilesystemTree>(path);
	current_root = filesystem_tree_->GetRoot();
	anal::AnalyzeFilesystemTree(filesystem_tree_.get());
}

void app::App::Ls(const std::vector<std::string>& args)
{
	//std::uint64_t min_size = 0;
	//std::uint64_t max_size = std::numeric_limits<std::uint64_t>::max();
	//
	//if (args.size() > 0)
	//{
	//	min_size = std::stoi(args[0]);
	//}
	//
	//if (args.size() > 1)
	//{
	//	max_size = std::stoi(args[1]);
	//}
	//
	//if (!filesystem_tree_)
	//{
	//	std::cout << "No scan has been performed yet! Use 'scan' first." << std::endl;
	//	return;
	//}
	
	const auto additional_spaces = [](std::uint64_t num) -> std::string
	{
		std::stringstream return_value;
		for (std::uint64_t i = 0; i < num; i++)
		{
			return_value << " ";
		}
		return return_value.str();
	};

	auto& folders = current_root->GetFolders();
	auto& files = current_root->GetFiles();

	std::cout << "--------------------------------------\n";
	std::cout << "Folders: \n";

	std::vector<fs_tree::display_info> info_vector;
	std::uint64_t longest_path = 0;

	auto root_info = current_root->GetDisplayInfo();
	longest_path = std::max(longest_path, static_cast<std::uint64_t>(root_info.path.size()));
	info_vector.push_back(std::move(root_info));

	for (const auto& folder : folders)
	{
		const auto info = folder->GetDisplayInfo();
		longest_path = std::max(longest_path, static_cast<std::uint64_t>(info.path.size()));
		info_vector.emplace_back(folder->GetDisplayInfo());
	}

	for (const auto& info : info_vector)
	{
		//if (info.size < min_size || info.size > max_size) continue;
		std::cout << "name: " << info.path << additional_spaces(longest_path - info.path.size()) << " | size: " << info.size << " " << info.unit << std::endl;
	}
	
	std::cout << "--------------------------------------\n";
	std::cout << "Files: \n";

	info_vector.clear();
	longest_path = 0;

	for (const auto& file : files)
	{
		const auto info = file->GetDisplayInfo();
		longest_path = std::max(longest_path, static_cast<std::uint64_t>(info.path.size()));
		info_vector.emplace_back(file->GetDisplayInfo());
	}

	for (const auto& info : info_vector)
	{
		//if (info.size < min_size || info.size > max_size) continue;
		std::cout << "name: " << info.path << additional_spaces(longest_path - info.path.size()) << " | size: " << info.size << " " << info.unit << std::endl;
	}
}

void app::App::Cd(const std::vector<std::string>& args)
{
	if (args.size() == 0)
	{
		std::cout << "No path provided!" << std::endl;
		return;
	}

	if (args.size() == 2)
	{
		if (args[0] == "..")
		{
			current_path_ = current_path_.parent_path();
			return;
		}
		else
		{
			auto new_path = current_path_ / args[0];
			if (std::filesystem::exists(new_path))
			{
				current_path_ = new_path;
			}
			else if (std::filesystem::exists(args[0]))
			{
				current_path_ = args[0];
			}
			else
			{
				std::cout << "Path does not exist!" << std::endl;
				return;
			}
		}
	}
}

void app::App::Rmdir(const std::vector<std::string>& args)
{
	if (args.size() == 2)
	{
		std::filesystem::remove_all(args[0]);
	}
}

std::vector<std::string> app::App::ParseCommand(const std::string& command)
{
	std::vector<std::string> return_value;
	std::string current;
	current.reserve(command.size());
	for (const auto& c : command)
	{
		if (c == ' ')
		{
			return_value.push_back(current);
			current.clear();
		}
		else
		{
			current.push_back(c);
		}
	}
	if (!current.empty())
	{
		return_value.push_back(current);
	}
	return return_value;
}

void app::App::RunCommand(const std::string& command, const std::vector<std::string>& args)
{
	const auto search = commands_.find(command);
	if (search == commands_.end())
	{
		return;
	}
	search->second.first(args);
}

app::App::App()
{
}

app::App::App(wchar_t* path)
{
	current_path_ = std::filesystem::path(path);
	std::cout << "current_path_: " << current_path_ << std::endl;
}

app::App::~App()
{
}

void app::App::Run()
{	
	std::cout << "Welcome to FolderScanner!" << std::endl;
	std::cout << "Type 'help' and than press 'enter' to see a list of all available commands with their descriptions." << std::endl;
	while (!app_finished_)
	{
		if (anal::ProcessingFinished())
		{
			if (const auto o = GetCommandAndArgs())
			{
				const auto [command, args] = o.value();
				RunCommand(command, args);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
