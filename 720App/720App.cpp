// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
#include "SImageButtonHover.h"
//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE 0
//#define SYSRES_TYPE 0
// #define RES_TYPE 0   //PE
// #define RES_TYPE 1   //ZIP
// #define RES_TYPE 2   //7z
// #define RES_TYPE 2   //文件
//去掉多项支持，以免代码显得混乱
#if (RES_TYPE==1)
#include "resprovider-zip\zipresprovider-param.h"
#else 
#if (RES_TYPE==2)
#include "resprovider-7zip\zip7resprovider-param.h"
#endif
#endif
#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

//定义唯一的一个R,UIRES对象,ROBJ_IN_CPP是resource.h中定义的宏。
ROBJ_IN_CPP

class SOUIEngine
{
private:
	SComMgr m_ComMgr = SComMgr(L"imgdecoder-gdip");
	SApplication *m_theApp;
	bool m_bInitSucessed;
public:
	SOUIEngine(HINSTANCE hInstance):m_theApp(NULL), m_bInitSucessed(false){
		
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
		BOOL bLoaded = FALSE;
		//使用Skia渲染界面
		bLoaded = m_ComMgr.CreateRender_Skia((IObjRef * *)& pRenderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
		//设置图像解码引擎。默认为GDIP。基本主流图片都能解码。系统自带，无需其它库
		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		bLoaded = m_ComMgr.CreateImgDecoder((IObjRef * *)& pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		m_theApp = new SApplication(pRenderFactory, hInstance);	
		m_bInitSucessed = (TRUE==bLoaded);
	};
	operator bool()const
	{
		return m_bInitSucessed;
	}
	//加载系统资源
	bool LoadSystemRes()
	{
		BOOL bLoaded = FALSE;

		//从DLL加载系统资源
		{
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (hModSysResource)
			{
				CAutoRefPtr<IResProvider> sysResProvider;
				CreateResProvider(RES_PE, (IObjRef * *)& sysResProvider);
				sysResProvider->Init((WPARAM)hModSysResource, 0);
				m_theApp->LoadSystemNamedResource(sysResProvider);
				FreeLibrary(hModSysResource);
			}
			else
			{
				SASSERT(0);
			}
		}

		return TRUE==bLoaded;
	}
	//加载用户资源
	bool LoadUserRes()
	{
		CAutoRefPtr<IResProvider>   pResProvider;
		BOOL bLoaded = FALSE;
#ifdef _DEBUG		
		//选择了仅在Release版本打包资源则在DEBUG下始终使用文件加载
		{
			CreateResProvider(RES_FILE, (IObjRef * *)& pResProvider);
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
		}
#else
		{
			CreateResProvider(RES_PE, (IObjRef * *)& pResProvider);
			bLoaded = pResProvider->Init((WPARAM)m_theApp->GetInstance(), 0);
			SASSERT(bLoaded);
		}
#endif
		m_theApp->InitXmlNamedID(namedXmlID, ARRAYSIZE(namedXmlID), TRUE);
		m_theApp->AddResProvider(pResProvider);
		return TRUE==bLoaded;
	}
	//加载LUA支持
	bool LoadLUAModule()
	{
		BOOL bLoaded =FALSE;
		CAutoRefPtr<SOUI::IScriptFactory> pScriptLuaFactory;
		bLoaded = m_ComMgr.CreateScrpit_Lua((IObjRef * *)& pScriptLuaFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
		m_theApp->SetScriptFactory(pScriptLuaFactory);
		return TRUE == bLoaded;
	}
	//加载多语言支持
	bool LoadMultiLangsModule()
	{
		BOOL bLoaded = FALSE;
		CAutoRefPtr<ITranslatorMgr> trans;
		bLoaded = m_ComMgr.CreateTranslator((IObjRef * *)& trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
		if (trans)
		{//加载语言翻译包
			m_theApp->SetTranslator(trans);
			pugi::xml_document xmlLang;
			if (m_theApp->LoadXmlDocment(xmlLang,  _T("translator:lang_cn")))
			{
				CAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				langCN->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
				trans->InstallTranslator(langCN);
			}
		}
		return TRUE == bLoaded;
	}
	//注册用户自定义皮肤和控件
	void Regitercustom()
	{
		m_theApp->RegisterWindowClass<SImageButtonHover>();//SImageButtonHover
	}

	~SOUIEngine()
	{
		if (m_theApp)
		{
			delete m_theApp;
			m_theApp = NULL;
		}
	}

	template<class MainWnd>
	int Run()
	{
		MainWnd dlgMain;
		dlgMain.Create(GetActiveWindow());
		dlgMain.SendMessage(WM_INITDIALOG);
		dlgMain.CenterWindow(dlgMain.m_hWnd);
		dlgMain.ShowWindow(SW_SHOWNORMAL);
		return m_theApp->Run(dlgMain.m_hWnd);
	}

	SApplication* GetApp()
	{
		return m_theApp;
	}
};
//debug时方便调试设置当前目录以便从文件加载资源
void AppSetDefaultDir()
{
	TCHAR szCurrentDir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));

	LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
#ifdef _DEBUG
	_tcscpy(lpInsertPos + 1, _T("..\\720App"));
#else
	_tcscpy(lpInsertPos + 1, _T("\0"));
#endif
	SetCurrentDirectory(szCurrentDir);
}

#include "../720Core/720CoreExport.h"
#include "../720Core/CApp.h"
#include "../720Core/CWindowsOpenGLView.h"

struct AppRunCallbackData
{
	SOUIEngine* souiEngine;
	CMainDlg* dlgMain;
};

int AppRunCallback(void* ptr, CWindowsOpenGLView* view, CWindowsGameRenderer* renderer) {
	auto* data = (AppRunCallbackData*)ptr;
	data->dlgMain->SetGameRenderer(renderer);
	data->dlgMain->SetView(view);
	data->dlgMain->ShowWindow(SW_SHOWNORMAL);

	view->SetWindowsMessageTarget(data->dlgMain->m_hWnd);
	return  data->souiEngine->GetApp()->Run(data->dlgMain->m_hWnd);
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

	CApp* app = nullptr;

	AppSetDefaultDir();
	AppSetUnhandledExceptionFilter();
	AppCreate((void**)&app);

	int nRet = 0;
	{
		SOUIEngine souiEngine(hInstance);
		if (souiEngine)
		{
			//加载系统资源
			souiEngine.LoadSystemRes();
			//加载用户资源
			souiEngine.LoadUserRes();
			//注册用户自定义皮肤/控件/布局/其它自定义
			souiEngine.Regitercustom();

			//创建主窗口
			CMainDlg dlgMain;

			AppRunCallbackData* runCallbackData = new AppRunCallbackData();
			runCallbackData->souiEngine = &souiEngine;
			runCallbackData->dlgMain = &dlgMain;

			if (app->Init())
			{
				SettingHlp* settings = app->GetSettings();

				dlgMain.Create(GetActiveWindow());
				dlgMain.SetWindowLong(GWL_STYLE, dlgMain.GetWindowLong(GWL_STYLE) ^ WS_OVERLAPPED);
				dlgMain.SendMessage(WM_INITDIALOG);
				dlgMain.CenterWindow(dlgMain.m_hWnd);

				app->SetAppRunCallback(runCallbackData, AppRunCallback);
				app->SetCurrentHWND(dlgMain.m_hWnd);
				app->Run(&nRet);
			}

			app->Destroy();

			delete runCallbackData;
		}
		else
		{
			nRet = -1;
			MessageBox(NULL, _T("无法正常初使化SOUI"), _T("错误"), MB_OK | MB_ICONERROR);
		}
	}

	AppFree(app);

	OleUninitialize();
	return nRet;
}
