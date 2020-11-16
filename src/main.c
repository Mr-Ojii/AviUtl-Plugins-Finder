#include <stdio.h>
#include <windows.h>
#include "aviutl_plugin_sdk/filter.h"
#include "aviutl_plugin_sdk/color.h"
#include "aviutl_plugin_sdk/input.h"
#include "aviutl_plugin_sdk/output.h"

typedef OUTPUT_PLUGIN_TABLE* (*GetOutputPluginTable)(void);
typedef INPUT_PLUGIN_TABLE* (*GetInputPluginTable)(void);
typedef COLOR_PLUGIN_TABLE* (*GetColorPluginTable)(void);
typedef FILTER_DLL* (*GetFilterPluginTable)(void);

void PluginsFinder(FILE* fp, const char directoryPath[MAX_PATH])
{
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	char search_name[MAX_PATH];
	strcpy(search_name, directoryPath);
	strcat(search_name, "\\*");

	hFind = FindFirstFile(search_name, &win32fd);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		return;
	}

	do {
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			//再帰探索
			char subFolderPath[MAX_PATH];
			strcpy(subFolderPath, win32fd.cFileName);
			if (strcmp(subFolderPath, ".") != 0 && strcmp(subFolderPath, "..") != 0)
			{
				char nextdirectory[MAX_PATH];
				strcpy(nextdirectory, directoryPath);
				strcat(nextdirectory, "\\");
				strcat(nextdirectory, subFolderPath);
				PluginsFinder(fp, nextdirectory);
			}
		}
		else
		{

			char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
			strcpy(path, directoryPath);
			strcat(path, "\\");
			strcat(path, win32fd.cFileName);

			_splitpath(path, drive, dir, fname, ext);

			if (strcmp(ext, ".auf") == 0 || strcmp(ext, ".aui") == 0 || strcmp(ext, ".auo") == 0 || strcmp(ext, ".auc") == 0 || strcmp(ext, ".aul") == 0)
			{
				HMODULE createwindowmodule = LoadLibrary(path);

				if (NULL != createwindowmodule)
				{

					if (strcmp(ext, ".auo") == 0)
					{
						// LoadFunction
						GetOutputPluginTable creation = (GetOutputPluginTable)GetProcAddress(createwindowmodule, "GetOutputPluginTable");

						if (creation != NULL)
						{
							OUTPUT_PLUGIN_TABLE* info = creation();
							fprintf(fp, ".auo,%s,%s\n", info->information, path);
						}
					}
					else if (strcmp(ext, ".aui") == 0)
					{
						// LoadFunction
						GetInputPluginTable creation = (GetInputPluginTable)GetProcAddress(createwindowmodule, "GetInputPluginTable");

						if (creation != NULL)
						{
							INPUT_PLUGIN_TABLE* info = creation();
							fprintf(fp, ".aui,%s,%s\n", info->information, path);
						}
					}
					else if (strcmp(ext, ".auc") == 0)
					{
						// LoadFunction
						GetColorPluginTable creation = (GetColorPluginTable)GetProcAddress(createwindowmodule, "GetColorPluginTable");

						if (creation != NULL)
						{
							COLOR_PLUGIN_TABLE* info = creation();
							fprintf(fp, ".auc,%s,%s\n", info->information, path);
						}
					}
					else if(strcmp(ext, ".auf") == 0)
					{
						// LoadFunction
						GetFilterPluginTable creation = (GetFilterPluginTable)GetProcAddress(createwindowmodule, "GetFilterTable");

						if (creation != NULL)
						{
							FILTER_DLL* info = creation();
							fprintf(fp, ".auf,%s,%s\n", info->information, path);
						}
					}
					else 
					{
						char string[1024];
						LoadStringA(createwindowmodule, 1, string, sizeof(string));
						fprintf(fp, ".aul,%s,%s\n", string, path);
					}

					FreeLibrary(createwindowmodule);
				}
			}
		}
	} while (FindNextFile(hFind, &win32fd));

	FindClose(hFind);
}

int main(int argc, char* argv[])
{
	//csvファイルを開く
	FILE* fp = fopen("PluginList.csv", "w");

	//開けた場合のみ
	if (fp != NULL)
	{
		//exeが存在しているディレクトリの判定
		char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], directorypath[MAX_PATH];
		if (GetModuleFileNameA(NULL, path, MAX_PATH) != 0)
		{
			_splitpath(path, drive, dir, NULL, NULL);

			strcpy(directorypath, drive);
			strcat(directorypath, dir);
			printf("Searching...");

			PluginsFinder(fp, directorypath);
			printf("\rFinished!   \n");
		}
		else 
		{
			fprintf(stderr, "Could not get exe file path.");
		}

		fclose(fp);
	}
	else 
	{
		//開けなかった場合、エラーを吐き、落とす。
		fprintf(stderr, "Could not open the csv file.");
		return 1;
	}

	return 0;
}
