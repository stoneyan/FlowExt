#include <stdio.h>
#ifdef WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif
#include "semantic.h"

void AnalyzeFile(char* file_name, int argc, char* argv[])
{
    //FILE* wfp = fopen("output.cpp", "w");

	try {
	CNamespace* pNamespace = semanticAnalyzeFile(file_name, argc, argv);
	std::string ret_s = pNamespace->toString(0);
	printf("%s\n", ret_s.c_str());
	}
	catch (std::string& s)
	{
		printf("analyzeFile failed, err=%s\n", s.c_str());
	}
}

int main(int argc, char* argv[])
{
    semanticInit();

    argc--;
    argv++;

    DIR *d;
    struct dirent *dir;
    d = opendir(".");

    while ((dir = readdir(d)) != NULL)
    {
        if (strcmp(dir->d_name, "test.cpp") != 0)
            continue;
        char* postfix = strrchr(dir->d_name, '.');
        if (postfix == NULL)
            continue;
        if (strcmp(postfix, ".cpp") == 0 || strcmp(postfix, "*.c") == 0)
            AnalyzeFile(dir->d_name, argc, argv);
    }
    closedir(d);

    //printf("===================print logic object tree=============\n");
    //dump_htmls();

    //printf("done\n");
    return 0;
}
