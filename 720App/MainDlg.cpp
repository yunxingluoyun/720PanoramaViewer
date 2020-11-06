// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "../720Core/CWindowsGameRenderer.h"
#include "../720Core/CWindowsOpenGLView.h"
#include "../720Core/SystemHelper.h"

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
}
CMainDlg::~CMainDlg()
{
}
BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	return 0;
}
//TODO:消息映射
void CMainDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}
void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}

	if (view)
		::MoveWindow(view->GetHWND(), 1, titleBarHeight, size.cx - 2, size.cy - toolBarHeight - titleBarHeight, FALSE);
}

void CMainDlg::OnKillFocus(HWND hWnd)
{
	if (view) view->SetToLowerFpsMode();
}
void CMainDlg::OnSetFocus(HWND hWnd)
{
	if (view) view->QuitLowerFpsMode();
}
void CMainDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT  nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	TCHAR szFileName[_MAX_PATH] = _T("");
	DWORD dwAttribute;

	if (nFileCount > 0) {
		::DragQueryFile(hDropInfo, 0, szFileName, sizeof(szFileName));
		dwAttribute = ::GetFileAttributes(szFileName);

		if ((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			gameRenderer->SetOpenFilePath(szFileName);
			gameRenderer->MarkShouldOpenFile();
		}
	}

	::DragFinish(hDropInfo);
}
void CMainDlg::OnCustomMessage(UINT uiMsg)
{
	switch (uiMsg)
	{
	case WM_CUSTOM_SHOW_ABOUTBOX: {
		//CAboutDialog::Show(m_hWnd);
		gameRenderer->NotifyAboutDialogClosed();
		break;
	}
	case WM_CUSTOM_SHOW_HELPBOX: {
		//CHelpDialog::Show(m_hWnd);
		gameRenderer->NotifyHelpDialogClosed();
		break;
	}
	case WM_CUSTOM_OPEN_FILE: {
		WCHAR strFilename[MAX_PATH] = { 0 };
		if (SystemHelper::ChooseOneFile(m_hWnd, NULL, L"请选择一个全景图", L"图片文件\0*.jpg;*.png;*.bmp\0所有文件(*.*)\0*.*\0\0\0",
			strFilename, NULL, strFilename, MAX_PATH)) {
			gameRenderer->SetOpenFilePath(strFilename);
			gameRenderer->MarkShouldOpenFile();
		}
		break;
	}
	case WM_CUSTOM_CMD_OPENFILE: {
		std::wstring path = CApp::Instance->GetSettings()->GetSettingStr(L"NextOpenFilePath", L"", 512);
		if (_waccess_s(path.c_str(), 0) == 0) {
			gameRenderer->SetOpenFilePath(path.c_str());
			gameRenderer->MarkShouldOpenFile();
		}
		break;
	}
	case WM_CUSTOM_DEL_FILE: {
		if (MessageBox(m_hWnd, L"你是否真的要删除该图像？\n删除后不能恢复！", L"警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
			gameRenderer->MarkCloseFile(true);
		}
	}
	default:
		break;
	}
}
void CMainDlg::SetView(CWindowsOpenGLView* view) {
	CRect rect = GetClientRect();
	::SetParent(view->GetHWND(), m_hWnd);
	::MoveWindow(view->GetHWND(), 1, titleBarHeight, rect.Width() - 2, rect.Height() - toolBarHeight - titleBarHeight, TRUE);

	this->view = view;
}
void CMainDlg::SetGameRenderer(CWindowsGameRenderer* gameRenderer) {
	this->gameRenderer = gameRenderer;
}