// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"
#include <shellapi.h>
#include <Shlwapi.h>
#include "../720Core/720Core.h"
#include "../720Core/CApp.h"
#include "../720Core/messages.h"

class CMainDlg : public SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	void OnKillFocus(HWND hWnd);
	void OnSetFocus(HWND hWnd);
	void OnDropFiles(HDROP hDropInfo);

	void OnCustomMessage(UINT uiMsg);

	void SetView(CWindowsOpenGLView* view);
	void SetGameRenderer(CWindowsGameRenderer* gameRenderer);

protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
	EVENT_MAP_END()
		
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_DROPFILES(OnDropFiles)
		if (uMsg >= WM_CUSTOM_MIN && uMsg <= WM_CUSTOM_MAX)
			OnCustomMessage(uMsg);
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:

	int titleBarHeight = 40;
	int toolBarHeight = 50;
	CWindowsOpenGLView* view = nullptr;
	CWindowsGameRenderer* gameRenderer = nullptr;

};
