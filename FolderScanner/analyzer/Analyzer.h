#ifndef ANALYZE_ANALYZER
#define ANALYZE_ANALYZER

#include "../fs_tree/FileSystemTree.h"
#include "../fs_tree/Folder.h"
#include <condition_variable>
namespace anal
{
	void AnalyzeFilesystemTree(fs_tree::FilesystemTree* filesystem_tree);
	void FinishLoadingFolders();
	bool LoadingFinished();
	bool ProcessingFinished();
	std::condition_variable& GetLoadingConditionVariable();
}

#endif // !ANALYZE_ANALYZER

