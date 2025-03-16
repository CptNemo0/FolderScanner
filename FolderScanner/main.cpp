#include <iostream>
#include "app/App.h"	
#include <filesystem>
#include <windows.h>

int main()
{
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	
	app::App app { buffer };

	app.Run();
	return 0;
}