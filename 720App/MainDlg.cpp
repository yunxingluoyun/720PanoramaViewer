// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	


#define TIMER_TIP_SHOW  2031
#define TIMER_TIP_CLOSE  2033
#define TIMER_LATE_INIT_GAME  2032
#define TIMER_NOTIFICATION_SHOW  2034

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
}
CMainDlg::~CMainDlg()
{

}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	shadowWindow = new CShadowWindow();
	shadowWindow->Create(m_hWnd);
	shadowWindow->SendMessage(WM_INITDIALOG);

	tipWind = new CTipWnd();
	tipWind->Create(m_hWnd);
	tipWind->SendMessage(WM_INITDIALOG);

	notifyWnd = new CNotifyWnd();
	notifyWnd->Create(m_hWnd);
	notifyWnd->SendMessage(WM_INITDIALOG);

	panoModeWnd = new CPanoModeWnd(this);
	panoModeWnd->Create(m_hWnd);
	panoModeWnd->SendMessage(WM_INITDIALOG);

	strTitle = SApplication::getSingletonPtr()->GetString(R.string.title);

	CApp* app = AppGetAppInstance();

	//MessageHandlers
	CCThreadMessageCenter* messageCenter = app->GetMessageCenter();
	uiThreadMessageHandler = messageCenter->CreateThreadMessageHandler(2);
	uiThreadMessageHandler->isUiThread = true;
	uiThreadMessageHandler->targetWindow = m_hWnd;
	messageCenter->RigisterThreadMessageHandler(2, uiThreadMessageHandler);

	//Settings
	SettingHlp* settings = app->GetSettings();
	fullscreen = settings->GetSettingBool(L"FullScreen", false);
	if (fullscreen) UpdateFullScreenState();
	else {
		bool max = settings->GetSettingBool(L"Maximize", false);
		if (!max) {
			int w = settings->GetSettingInt(L"Width", 1024);
			int h = settings->GetSettingInt(L"Height", 768);
			int cw = GetSystemMetrics(SM_CXSCREEN);
			int ch = GetSystemMetrics(SM_CYSCREEN);
			if (w <= 800) w = 1024;
			if (h <= 600) h = 768;

			SetWindowPos(0, (cw - w) / 2, (ch - h) / 2, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else OnMaximize();
	}

	ResizeShadow();
	ResizeNotifyWnd();

	//Eles
	pWindowWelecome = FindChildByID(1);
	pWindowMain = FindChildByID(2);
	pWindowError = FindChildByID(3);
	pWindowError = FindChildByID(3);
	pTextTitle = FindChildByName2<SStatic>("text_title"); 
	pTextFileError = FindChildByName2<SStatic>("text_file_error");
	pBtnPanoMode = FindChildByName2<SImageButton>("btn_pano_mode");
	pBtnFullscreenMenu = FindChildByName2<SImageButton>("btn_fullscreen_menu");
	((SImageButtonHover*)pBtnPanoMode)->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_scale_100")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_full_screen")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_zoom_out")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_zoom_in")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_left_img")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_right_img")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_info")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_open_with")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_delete")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_album")->SetHoverCheckReceiver(m_hWnd);

	return TRUE;
}
void CMainDlg::OnClose()
{
	CRect rect = GetWindowRect();
	SettingHlp* settings = AppGetAppInstance()->GetSettings();
	settings->SetSettingBool(L"FullScreen", fullscreen);
	settings->SetSettingInt(L"Width", rect.Width());
	settings->SetSettingInt(L"Height", rect.Height());
	settings->SetSettingBool(L"Maximize", ismaxsize);

	if (gameRenderer) 
		gameRenderer->MarkCloseFile(false);
	if (view) 
		view->SetEnabled(false);
	if (uiThreadMessageHandler) {
		uiThreadMessageHandler = nullptr;
		AppGetAppInstance()->GetMessageCenter()->UnRigisterThreadMessageHandler(2);
	}
	if (tipWind)
		tipWind->DestroyWindow();
	if (shadowWindow)
		shadowWindow->DestroyWindow();
	if (panoModeWnd)
		panoModeWnd->DestroyWindow();
	if (notifyWnd)
		notifyWnd->DestroyWindow();

	SNativeWnd::DestroyWindow();
}
void CMainDlg::OnDestroy()
{
	if (view) {
		view->Destroy();
		view = nullptr;
	}

	PostQuitMessage(0);
}

void CMainDlg::OnShow(BOOL show, int s)
{
	if(shadowWindow)
		shadowWindow->ShowWindow(show ? SW_SHOW : SW_HIDE);
}
void CMainDlg::OnTimer(UINT_PTR id)
{
	switch (id)
	{
	case TIMER_TIP_SHOW: {
		KillTimer(TIMER_TIP_SHOW);
		if (tooltipShow) {
			CRect rect = toolTipCurrent->GetWindowRect();
			ClientToScreen(rect);
			CRect rectTip = tipWind->GetWindowRect();
			tipWind->SetWindowPos(HWND_TOP,
				rect.left - (rectTip.Width() - rect.Width()) / 2,
				rect.top - rectTip.Height() + 10,
				0, 0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		break;
	}
	case TIMER_NOTIFICATION_SHOW: {
		KillTimer(TIMER_NOTIFICATION_SHOW);
		CRect rect = GetWindowRect();
		CRect rectTip = notifyWnd->GetWindowRect();
		ClientToScreen(rect);
		notifyWnd->Show(
			rect.left + (rect.Width() - rectTip.Width()) / 2,
			rect.top + (rect.Height() - rectTip.Height()) / 2
		);
		break;
	}
	case TIMER_TIP_CLOSE: {
		KillTimer(TIMER_TIP_CLOSE);
		if (!tooltipShow)
			tipWind->Hide();
		break;
	}
	case TIMER_LATE_INIT_GAME: {
		KillTimer(TIMER_LATE_INIT_GAME);
		UpdatePanoButtonState();
		break;
	}
	default:
		SHostWnd::OnTimer(id);
		break;
	}
}
void CMainDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

}
void CMainDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{

}
void CMainDlg::OnRenderKey(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (view) ::PostMessage(view->GetHWND(), msg, wParam, lParam);
}
void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	SWindow* pBtnMax = FindChildByName(L"btn_max");
	SWindow* pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

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

	ismaxsize = nType == SIZE_MAXIMIZED;

	//阴影窗口
	if (shadowWindow) {
		shadowWindow->ShowWindow(
			(nType != SIZE_MINIMIZED && nType != SIZE_MAXIMIZED && nType != SIZE_MAXHIDE) ? SW_SHOW : SW_HIDE
		);
		ResizeShadow();
	}
	if (notifyWnd) ResizeNotifyWnd();

	//渲染窗口
	if (view) {
		ResizeView(size.cx, size.cy);
		::SendMessage(view->GetHWND(), WM_SIZE, nType, NULL);
	}
}
void CMainDlg::OnMove(CPoint pos)
{
	//阴影窗口
	if (shadowWindow) {
		shadowWindow->SetWindowPos(0, pos.x - shadowRadius, pos.y - shadowRadius, 0, 0, SWP_NOSIZE |
			SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	if (notifyWnd) {
		notifyWnd->SetWindowPos(0, pos.x, pos.y, 0, 0, SWP_NOSIZE |
			SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
void CMainDlg::OnKillFocus(HWND hWnd)
{
	if (isShowToolsForFocus) {
		isShowToolsForFocus = false;
		return;
	}
	if (view) view->SetToLowerFpsMode();
}
void CMainDlg::OnSetFocus(HWND hWnd)
{
	if (shadowWindow)
		::SetWindowPos(shadowWindow->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	if (notifyWnd)
		::SetWindowPos(notifyWnd->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	if (view) 
		view->QuitLowerFpsMode();
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
	case WM_CUSTOM_CMD_OPENFILE: {
		std::wstring path = AppGetAppInstance()->GetSettings()->GetSettingStr(L"NextOpenFilePath", L"", 512);
		if (_waccess_s(path.c_str(), 0) == 0) {
			gameRenderer->SetOpenFilePath(path.c_str());
			gameRenderer->MarkShouldOpenFile();
		}
		break;
	}
	default:
		break;
	}
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

void CMainDlg::OnMainMenu()
{
	SWindow* pBtn = FindChildByName(L"btn_menu");
	CRect rect = pBtn->GetWindowRect();
	ClientToScreen(rect);

	SMenuEx menu;
	if (menu.LoadMenu(_T("SMenuEx:menu_main")))
	{
		SMenuExItem* closeFileItem = menu.GetMenuItem(2);
		SMenuExItem* openWithItem = menu.GetMenuItem(4);
		SMenuExItem* snapShortWithItem = menu.GetMenuItem(5);
		SMenuExItem* showInfoWithItem = menu.GetMenuItem(11);
		closeFileItem->EnableWindow(isFileOpened);
		openWithItem->EnableWindow(isFileOpened);
		snapShortWithItem->EnableWindow(isFileOpened);
		showInfoWithItem->EnableWindow(isFileOpened);

		int id = menu.TrackPopupMenu(TPM_RETURNCMD, rect.left - 7, rect.top + rect.Height() - 7, m_hWnd);
		switch (id)
		{
		case 1: OnOpenFile();  break;
		case 2: OnCloseFile(); break;
		case 3: OnImportFile(); break;
		case 4: OnOpenWith(); break;
		case 5: OnSnapshot(); break;
		case 6: OnSettings(); break;
		case 7: OnFeedBack(); break;
		case 8: OnAbout(); break;
		case 9: OnClose(); break;
		case 10: OnHelp(); break;
		case 11: OnShowImageInfo(); break;
		default: break;
		}
	}
}
void CMainDlg::OnRightMenu()
{
	POINT pt;
	GetCursorPos(&pt);
	SMenuEx menu;
	if (menu.LoadMenu(_T("SMenuEx:menu_right")))
	{
		SMenuExItem* fullScreenItem = menu.GetMenuItem(14);
		fullScreenItem->SetWindowText(fullscreen ? L"退出全屏" : L"全屏");

		int id = menu.TrackPopupMenu(TPM_RETURNCMD, pt.x - 7, pt.y - 7, m_hWnd);
		switch (id)
		{
		case 1: OnOpenFile();  break;
		case 2: OnCloseFile(); break;
		case 3: OnPrev(); break;
		case 4: OnNext(); break;
		case 5: {
			const wchar_t* path = gameRenderer->GetCurrentFilePath();
			if (wcscmp(path, L"") != 0) {
				SystemHelper::ShellCopyFile(m_hWnd, path);
				ShowBigTip(L"图片已经复制");
			}
			break;
		}
		case 6: {
			const wchar_t* path = gameRenderer->GetCurrentFilePath();
			if (wcscmp(path, L"") != 0) {
				SystemHelper::CopyToClipBoard(path);
				ShowBigTip(L"图片路径已经复制到剪贴板");
			}
			break;
		}
		case 7: OnDelete(); break;
		case 8: {
			const wchar_t* path = gameRenderer->GetCurrentFilePath();
			if (wcscmp(path, L"") != 0)
				SystemHelper::ShowInExplorer(path);
			break;
		}
		case 9: OnOpenWith(); break;
		case 10: OnSnapshot(); break;
		case 11: OnShowImageInfo(); break;
		case 12: OnSettings(); break;
		case 13: OnClose(); break;
		case 14: 
			fullscreen = !fullscreen;
			UpdateFullScreenState();
			break;
		default: break;
		}
	}
}

void CMainDlg::SetView(CWindowsOpenGLView* view) {
	this->view = view;
	CRect rect = GetClientRect();
	ResizeView(rect.Width(), rect.Height());
}
void CMainDlg::SetGameRenderer(CWindowsGameRenderer* gameRenderer) {
	this->gameRenderer = gameRenderer;
	gameRenderer->SetFileStatusChangedCallback(OnGameFileStatusChangedCallback, this);
	gameRenderer->SetSampleEventCallback(OnGameSampleEventCallback, this);
	SetTimer(TIMER_LATE_INIT_GAME, 1000);
}

void CMainDlg::ResizeView(int w, int h) {
	if (view) {
		if(fullscreen) ::MoveWindow(view->GetHWND(), 1, 1, w - 2, h - 2, FALSE);
		else ::MoveWindow(view->GetHWND(), 1, titleBarHeight, w - 2, h - toolBarHeight - titleBarHeight, FALSE);
	}
}
void CMainDlg::ResizeShadow() {
	CRect rect = GetWindowRect();
	shadowWindow->SetWindowPos(0,0,0, 
		rect.Width() + shadowRadius * 2, rect.Height() + shadowRadius * 2, 
		SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}
void CMainDlg::ResizeNotifyWnd() {
	CRect rect = GetWindowRect();
	notifyWnd->SetWindowPos(0, 0, 0,
		rect.Width(), rect.Height(),
		SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}
void CMainDlg::UpdateFullScreenState() {
	gameRenderer->SetRenderQuitFullScreenButton(fullscreen);

	int cx = GetSystemMetrics(SM_CXSCREEN), cy = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen) {

		CRect rect = GetWindowRect();
		lastSize = glm::vec2(rect.Width(), rect.Height());
		SetWindowPos(HWND_NOTOPMOST, 0, 0, cx, cy, SWP_NOACTIVATE);
		rect = GetWindowRect();
		ResizeView(rect.Width(), rect.Height());
	}
	else {
		if (lastSize.x < 500)  lastSize.x = 500;
		if (lastSize.y < 400) lastSize.y = 400;

		ResizeView((int)lastSize.x, (int)lastSize.y);
		SetWindowPos(HWND_NOTOPMOST, (cx - (int)lastSize.x) / 2, (cy - (int)lastSize.y) / 2, (int)lastSize.x, (int)lastSize.y,
			SWP_NOACTIVATE);
		SNativeWnd::Invalidate();
	}
}
void CMainDlg::UpdatePanoButtonState() {
	switch (gameRenderer->GetMode())
	{
	case PanoramaMode::PanoramaCylinder:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_cylinder");
		break;
	case PanoramaMode::PanoramaAsteroid:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_asteroid");
		break;
	case PanoramaMode::PanoramaFull360:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_360");
		break;
	case PanoramaMode::PanoramaOuterBall:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_outball");
		break;
	case PanoramaMode::PanoramaSphere:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_sphere");
		break;
	default:
	case PanoramaMode::PanoramaFullOrginal:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_orginal");
		break;
	}
}
void CMainDlg::ShowBigTip(SStringW str, int showTime) {
	notifyWnd->SetTip(str, showTime);
	SetTimer(TIMER_NOTIFICATION_SHOW, 200);
}

//操作处理
//*********************************

void CMainDlg::OnOpenFile() {
	WCHAR strFilename[MAX_PATH] = { 0 };
	if (SystemHelper::ChooseOneFile(m_hWnd, NULL, L"请选择一个全景图", L"图片文件\0*.jpg;*.png;*.bmp\0所有文件(*.*)\0*.*\0\0\0",
		strFilename, NULL, strFilename, MAX_PATH)) {
		gameRenderer->SetOpenFilePath(strFilename);
		gameRenderer->MarkShouldOpenFile();
	}
}
void CMainDlg::OnCloseFile() { gameRenderer->MarkCloseFile(false); }
void CMainDlg::OnImportFile() {

}
void CMainDlg::OnFullScreen() {
	if (isFileOpened) {
		fullscreen = true;
		UpdateFullScreenState();
	}
}
void CMainDlg::OnOpenWith() {
	gameRenderer->OpenFileAs();
}
void CMainDlg::OnSnapshot() {
	ShowBigTip(L"OnSnapshot!");
}
void CMainDlg::OnZoomIn() { 
	if (gameRenderer->ZoomIn())
		ShowBigTip(L"已经缩放到最大啦", 1000);
}
void CMainDlg::OnZoomOut() { 
	if (gameRenderer->ZoomOut())
		ShowBigTip(L"已经缩放到最小啦", 1000);
}
void CMainDlg::OnZoomReset() { gameRenderer->ZoomBest(); }
void CMainDlg::OnDelete() {
	if (MessageBox(m_hWnd, L"你是否真的要删除该图像？\n删除后不能恢复！", L"警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
		gameRenderer->MarkCloseFile(true);
	}
}
void CMainDlg::OnShowImageInfo() {

}
void CMainDlg::OnShowAlbum()
{

}
void CMainDlg::OnChangePanoMode()
{
	if (panoModeWnd->IsWindowVisible())
		panoModeWnd->Hide();
	else {
		isShowToolsForFocus = true;
		CRect rect = pBtnPanoMode->GetWindowRect();
		ClientToScreen(rect);
		CRect rectTip = panoModeWnd->GetWindowRect();
		panoModeWnd->Show(
			rect.left - (rectTip.Width() - rect.Width()) / 2,
			rect.top - rectTip.Height() + 10
		);
	}
}
void CMainDlg::OnNext()
{
}
void CMainDlg::OnPrev()
{
}
void CMainDlg::OnPanoModeChanged(PanoramaMode mode) {
	gameRenderer->SetMode(mode);
	UpdatePanoButtonState();
}
void CMainDlg::OnToolButtonMouse(SImageButtonHover* ctl, bool enter)
{
	if (enter) {
		tooltipShow = TRUE;
		toolTipCurrent = ctl;
		tipWind->SetTip(ctl->GetMyToolTip());
		SetTimer(TIMER_TIP_SHOW, 50);
	}
	else {
		tooltipShow = FALSE;
		SetTimer(TIMER_TIP_CLOSE, 100);
	}
}

//打开子窗口
//*********************************

void CMainDlg::OnSettings() {

}
void CMainDlg::OnAbout() {
	CAboutDlg aboutDlg;
	aboutDlg.Create(m_hWnd);
	aboutDlg.SendMessage(WM_INITDIALOG);
	aboutDlg.CenterWindow(aboutDlg.m_hWnd);
	aboutDlg.DoModal(m_hWnd);
}
void CMainDlg::OnHelp() {
	CHelpDlg helpDlg;
	helpDlg.Create(m_hWnd);
	helpDlg.SendMessage(WM_INITDIALOG);
	helpDlg.CenterWindow(helpDlg.m_hWnd);
	helpDlg.DoModal(m_hWnd);
}
void CMainDlg::OnFeedBack() {

}

//回调处理
//*********************************
void CMainDlg::OnThreadMessage(int handlerId)
{
	if (uiThreadMessageHandler != nullptr && handlerId == 2)
		uiThreadMessageHandler->RunNext();
}
void CMainDlg::OnGameFileStatusChanged(bool isOpen, int status) {

	pWindowWelecome->SetVisible(!isOpen, true);
	pWindowMain->SetVisible(isOpen, true);
	pBtnFullscreenMenu->SetVisible(isOpen, true);
	view->SetVisible(isOpen);

	isFileOpened = isOpen;

	if (isOpen) {
		LPCWSTR currentFileInfoTitle = gameRenderer->GetCurrentFileInfoTitle();
		SNativeWnd::SetWindowText(currentFileInfoTitle);
		pTextTitle->SetWindowText(currentFileInfoTitle);
	}
	else {
		pTextTitle->SetWindowText(strTitle);
		SNativeWnd::SetWindowText(strTitle);
		if (status == GAME_FILE_OPEN_FAILED) {
			pWindowWelecome->SetVisible(false, true);
			pWindowError->SetVisible(true, true);
			pTextFileError->SetWindowText(gameRenderer->GetFileLastError());
		}
	}
}
void CMainDlg::OnGameFileStatusChangedCallback(void* data, bool isOpen, int status)
{
	auto* thisPtr = (CMainDlg*)data;
	thisPtr->OnGameFileStatusChanged(isOpen, status);
}
void CMainDlg::OnGameSampleEventCallback(void* data, int eventCode, void* param)
{
	auto* thisPtr = (CMainDlg*)data;
	switch (eventCode)
	{
	case GAME_EVENT_QUIT_FULLSCREEN: {
		if (thisPtr->fullscreen) {
			thisPtr->fullscreen = false;
			thisPtr->UpdateFullScreenState();
		}
		break;
	}
	case GAME_EVENT_GO_FULLSCREEN: {
		thisPtr->fullscreen = !thisPtr->fullscreen;
		thisPtr->UpdateFullScreenState();
		break;
	}
	case GAME_EVENT_SHOW_RIGHT_MENU: {
		thisPtr->OnRightMenu();
		break;
	}
	case GAME_EVENT_FILE_DELETE_BACK: {
		if ((bool)param) thisPtr->ShowBigTip(L"文件已刪除", 1700);
		else thisPtr->ShowBigTip(L"文件删除失败，文件可能被其他软件占用", 3500);
		break;
	}
	default:
		break;
	}
}




