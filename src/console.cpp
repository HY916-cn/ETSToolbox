#include "console.h"
#include<cstdio>

void createConsole()
{
	if (AllocConsole()) {
		SetConsoleTitle(L"ETSToolbox");
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
	}
}
