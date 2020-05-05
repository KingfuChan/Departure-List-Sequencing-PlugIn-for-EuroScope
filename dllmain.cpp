// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <EuroScopePlugIn.h>
#include "SequencingPlugIn.h"
using namespace EuroScopePlugIn;

//用于EuroScope加载插件
CSequencingPlugIn* pMyPlugIn = nullptr;

void __declspec (dllexport)
EuroScopePlugInInit(CPlugIn** ppPlugInInstance)
{
    // allocate
    *ppPlugInInstance = pMyPlugIn = new CSequencingPlugIn;
}

void __declspec (dllexport)
EuroScopePlugInExit(void)
{
    delete pMyPlugIn;
}
//以上为ES特有
