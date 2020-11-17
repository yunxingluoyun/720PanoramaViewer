// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "../720Core/PathHelper.h"
#include "FormatString.h"

#define TIMER_TIP_SHOW  2031
#define TIMER_TIP_CLOSE  2033
#define TIMER_LATE_INIT_GAME  2032
#define TIMER_NOTIFICATION_SHOW  2034
#define TIMER_LOADING_UPDATE_PEC  2035

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
		else shouldShowMax = true;
	}

	ResizeShadow();
	ResizeNotifyWnd();

	//Eles
	pWindowWelecome = FindChildByID(1);
	pWindowMain = FindChildByID(2);
	pWindowError = FindChildByID(3);
	pWindowImageInfo = FindChildByID(4);
	pWindowAlbum = FindChildByName(L"wnd_album");
	pWndAlbumTitle = FindChildByName(L"wnd_album_title");
	pWndAlbumTitleSelect = FindChildByName(L"wnd_album_title_select");
	pWndAlbumSelectTools = FindChildByName(L"wnd_album_select_tools");
	pWndCon = FindChildByName(L"wnd_con"); 
	
	pTextTitle = FindChildByName2<SStatic>("text_title"); 
	pTextFileError = FindChildByName2<SStatic>("text_file_error");
	pBtnPanoMode = FindChildByName2<SImageButtonHover>("btn_pano_mode");
	pBtnFullscreenMenu = FindChildByName2<SImageButton>("btn_fullscreen_menu");
	pBtnPanoMode->SetHoverCheckReceiver(m_hWnd);
	pBtnLeftImg = FindChildByName2<SImageButtonHover>("btn_left_img");
	pBtnRightImg = FindChildByName2<SImageButtonHover>("btn_right_img");
	pBtnInfo = FindChildByName2<SImageButtonHover>("btn_info");
	pBtnOpenWith = FindChildByName2<SImageButtonHover>("btn_open_with");
	pBtnDelete = FindChildByName2<SImageButtonHover>("btn_delete");
	pBtnAlbumCheckAll = FindChildByName2<SImageButton>("btn_album_check_all");

	pLstViewMain = FindChildByName2<SListView>("lv_main");
	if (pLstViewMain)
	{
		mainAdapter = new CMainAlbumAdapter(&mainAlbumItems);
		mainAdapter->SetCheckItemChangeCallback(AlbumCheckItemChanged, this);

		ILvAdapter* pAdapter = mainAdapter;

		pLstViewMain->GetEventSet()->subscribeEvent(EventLVSelChanged::EventID, Subscriber(&CMainDlg::OnMainSelectionItemChanged, this));
		pLstViewMain->SetAdapter(pAdapter);

		pAdapter->Release();
	}

	pTextAlbumSelectTitle = FindChildByName2<SStatic>("text_album_select_title");
	pTextImageInfoFullPath = FindChildByName2<SStatic>("text_image_info_full_path");
	pTextImageInfoFileSize = FindChildByName2<SStatic>("text_image_info_file_size");
	pTextImageInfoResolution = FindChildByName2<SStatic>("text_image_info_resolution");
	pTextImageInfoShootingDate = FindChildByName2<SStatic>("text_image_info_shooting_date");
	pTextImageInfoModifyDate = FindChildByName2<SStatic>("text_image_info_modify_date");
	pTextImageInfoShutterTime = FindChildByName2<SStatic>("text_image_info_shutter_time");
	pTextImageInfoExposureBiasValue = FindChildByName2<SStatic>("text_image_info_exposure_bias_value");
	pTextImageInfoIsoSensitivity = FindChildByName2<SStatic>("text_image_info_iso_sensitivity");
	pTextImageInfoCamera = FindChildByName2<SStatic>("text_image_info_camera");
	pTextImageInfoFocalLength = FindChildByName2<SStatic>("text_image_info_focal_length");

	FindChildByName2<SImageButtonHover>("btn_scale_100")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_full_screen")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_zoom_out")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_zoom_in")->SetHoverCheckReceiver(m_hWnd);
	pBtnLeftImg->SetHoverCheckReceiver(m_hWnd);
	pBtnRightImg->SetHoverCheckReceiver(m_hWnd);
	pBtnInfo->SetHoverCheckReceiver(m_hWnd);
	pBtnOpenWith->SetHoverCheckReceiver(m_hWnd);
	pBtnDelete->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_album")->SetHoverCheckReceiver(m_hWnd);


	LoadAlbumList();

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

	SaveAlbumList();

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

	ClearAlbumList();
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
	if (show) {
		if (isFirstShow) {
			isFirstShow = false;
			if(shouldShowMax) ShowWindow(SW_MAXIMIZE);
		}
	}
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
		notifyWnd->Show();
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
	case TIMER_LOADING_UPDATE_PEC: {
		SStringW p = L"正在加载 ";
		p += gameRenderer->GetCurrentFileLoadingPrecent();
		notifyWnd->SetTip(p, 1000);
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
		view->SetEnabled(false);
		ResizeView(size.cx, size.cy);
		::SendMessage(view->GetHWND(), WM_SIZE, nType, NULL);
		view->SetEnabled(true);
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

	if (nFileCount == 1) {
		::DragQueryFile(hDropInfo, 0, szFileName, sizeof(szFileName));
		dwAttribute = ::GetFileAttributes(szFileName);

		if ((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			gameRenderer->SetOpenFilePath(szFileName);
			gameRenderer->MarkShouldOpenFile();

			AlbumListAdd(szFileName, &currentItem);
		}
	}
	else if (nFileCount > 1) {

		int count = 0;
		for (size_t i = 0; i < nFileCount; i++) {
			::DragQueryFile(hDropInfo, i, szFileName, sizeof(szFileName));
			dwAttribute = ::GetFileAttributes(szFileName);

			if ((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!AlbumListAdd(szFileName))
					count++;
			}
		}

		ShowBigTip(FormatString(L"成功添加 %d 张全景图！", count).c_str(), 4000);
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

#ifndef _DEBUG
		SMenuExItem* testtem = menu.GetMenuItem(12);
		testtem->SetVisible(false);
#endif

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
		case 12: {
			break;
		}
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
		case 8: OnShowImageDir();  break;
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
		else {
			int left = 1, wf = w - 2;
			if (isShowAightAlbum)
				wf -= rightWndWidth;
			if (isShowImageInfo) {
				left += leftWndWidth;
				wf -= leftWndWidth;
			}
			::MoveWindow(view->GetHWND(), left, titleBarHeight, wf, h - toolBarHeight - titleBarHeight, FALSE);
		}
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
void CMainDlg::UpdateToolButtonState() {
	pBtnInfo->EnableWindow(isFileOpened, true);
	pBtnOpenWith->EnableWindow(isFileOpened, true);
	pBtnDelete->EnableWindow(isFileOpened, true);
}
void CMainDlg::UpdateLoadingState() {
	pBtnLeftImg->EnableWindow(!isLoading, true);
	pBtnRightImg->EnableWindow(!isLoading, true);
	if (isLoading) {
		SetTimer(TIMER_LOADING_UPDATE_PEC, 200);
		notifyWnd->SetTip(L"正在加载中...", 1000);
		notifyWnd->ShowWindow(SW_SHOW);
	}
	else {
		KillTimer(TIMER_LOADING_UPDATE_PEC);
		notifyWnd->ShowWindow(SW_HIDE);
	}
}
void CMainDlg::UpdateLeftImageInfoState() {
	CRect rc = GetWindowRect();
	ResizeView(rc.Width(), rc.Height());
	pWindowImageInfo->SetVisible(isShowImageInfo, true);
	SNativeWnd::Invalidate();
}
void CMainDlg::UpdateRightAlbumInfoState() {
	CRect rc = GetWindowRect();
	ResizeView(rc.Width(), rc.Height());
	pWindowAlbum->SetVisible(isShowAightAlbum, true);
	pWndCon->SetAttribute(L"pos", isShowAightAlbum ? L"2,38,-327,-2" : L"2,38,-2,-2");
	SNativeWnd::Invalidate();
}
void CMainDlg::ShowBigTip(SStringW str, int showTime) {
	notifyWnd->SetTip(str, showTime);
	SetTimer(TIMER_NOTIFICATION_SHOW, 200);
}
void CMainDlg::DoLoadImageInfo(int id) {
	if (id == 1) {
		ResetImageInfo();

		pTextImageInfoFullPath->SetWindowText(gameRenderer->GetCurrentFilePath());
		pTextImageInfoFileSize->SetWindowText(gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_FILE_SIZE));
		pTextImageInfoResolution->SetWindowText(gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_RESOLUTION));
		pTextImageInfoModifyDate->SetWindowText(gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_DATE));
	}
	else if (id == 2) {
		const wchar_t* str;
		str = gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_SHOOTING_DATE);
		pTextImageInfoShootingDate->SetWindowText(wcscmp(str, L"") == 0 ? L"未知" : str);
		str = gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_SHUTTER_TIME);
		pTextImageInfoShutterTime->SetWindowText(gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_SHUTTER_TIME));
		str = gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_EXPOSURE_BIAS_VALUE);
		pTextImageInfoExposureBiasValue->SetWindowText(wcscmp(str, L"") == 0 ? L"未知" : str);
		str = gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_ISO_SENSITIVITY);
		pTextImageInfoIsoSensitivity->SetWindowText(wcscmp(str, L"") == 0 ? L"未知" : str);
		str = gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_CAMERA);
		pTextImageInfoCamera->SetWindowText(wcscmp(str, L" ()") == 0 ? L"未知" : str);
		str = gameRenderer->GetCurrentFileInfo(GAME_IMAGE_INFO_FOCAL_LENGTH);
		pTextImageInfoFocalLength->SetWindowText(wcscmp(str, L"") == 0 ? L"未知" : str);
	}
}
void CMainDlg::ResetImageInfo() {
	pTextImageInfoFullPath->SetWindowText(L"未知");
	pTextImageInfoFileSize->SetWindowText(L"未知");
	pTextImageInfoResolution->SetWindowText(L"未知");
	pTextImageInfoModifyDate->SetWindowText(L"未知");
	pTextImageInfoShootingDate->SetWindowText(L"未知");
	pTextImageInfoShutterTime->SetWindowText(L"未知");
	pTextImageInfoExposureBiasValue->SetWindowText(L"未知");
	pTextImageInfoIsoSensitivity->SetWindowText(L"未知");
	pTextImageInfoCamera->SetWindowText(L"未知");
	pTextImageInfoFocalLength->SetWindowText(L"未知");
}

//操作处理
//*********************************

void CMainDlg::OnOpenFile() {
	WCHAR strFilename[MAX_PATH] = { 0 };
	if (SystemHelper::ChooseOneFile(m_hWnd, NULL, L"请选择一个全景图", L"图片文件\0*.jpg;*.jpeg;*.png;*.bmp\0所有文件(*.*)\0*.*\0\0\0",
		strFilename, NULL, strFilename, MAX_PATH)) {
		gameRenderer->SetOpenFilePath(strFilename);
		gameRenderer->MarkShouldOpenFile();

		AlbumListAdd(strFilename, &currentItem);
		mainAdapter->notifyDataSetChanged();
	}
}
void CMainDlg::OnCloseFile() { gameRenderer->MarkCloseFile(false); }
void CMainDlg::OnImportFile() {
	MultiFileSelectResult* result = SystemHelper::ChooseMultiFile(m_hWnd, NULL, L"请选择一些全景图导入到图库中", L"图片文件\0*.jpg;*.jpeg;*.png;*.bmp\0所有文件(*.*)\0*.*\0\0\0", NULL, NULL);
	if (result) {

		int count = 0;
		for (size_t i = 0; i < result->fileCount; i++) {
			std::wstring path = result->dir;
			path += L"\\";
			path += result->files[i];
			if (AlbumListAdd(path.c_str()))
				count++;
		}

		mainAdapter->notifyDataSetChanged();

		ShowBigTip(FormatString(L"成功添加 %d 张全景图！", count).c_str(), 4000);
		SystemHelper::DeleteMultiFilResult(result);
	}
}
void CMainDlg::OnFullScreen() {
	if (isFileOpened) {
		fullscreen = true;
		UpdateFullScreenState();
	}
}
void CMainDlg::OnOpenWith() {
	const wchar_t *path = gameRenderer->GetOpenFilePath();
	if (wcscmp(path, L"") != 0)
		SystemHelper::OpenAs(path);
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
void CMainDlg::OnShowImageDir()
{
	const wchar_t* path = gameRenderer->GetCurrentFilePath();
	if (wcscmp(path, L"") != 0)
		SystemHelper::ShowInExplorer(m_hWnd, path);
}
void CMainDlg::OnShowImageFileInfo()
{
	const wchar_t* path = gameRenderer->GetCurrentFilePath();
	if (wcscmp(path, L"") != 0)
		SystemHelper::ShowFileProperties(m_hWnd , path);
}
void CMainDlg::OnShowImageInfo() {
	isShowImageInfo = !isShowImageInfo;
	UpdateLeftImageInfoState();
}
void CMainDlg::OnShowAlbum()
{
	isShowAightAlbum = !isShowAightAlbum;
	UpdateRightAlbumInfoState();
}
void CMainDlg::OnAlbumCheckMode()
{
	mainAdapter->SetCheckMode(true);
	pWndAlbumTitleSelect->SetVisible(true, true);
	pWndAlbumTitle->SetVisible(false, true);
	pWndAlbumSelectTools->SetVisible(true, true);
	mainAdapter->notifyDataSetChanged();
}
void CMainDlg::OnAlbumExitCheckMode()
{
	mainAdapter->SetCheckMode(false);
	pWndAlbumTitleSelect->SetVisible(false, true);
	pWndAlbumTitle->SetVisible(true, true);
	pWndAlbumSelectTools->SetVisible(false, true);
	mainAdapter->CheckNone();
	pLstViewMain->SetSel(mainAdapter->GetItemIndex(currentItem), true);
}
void CMainDlg::OnAlbumCheckAll()
{
	if (mainAdapter->GetCheckItemCount() == mainAlbumItems.size())
		mainAdapter->CheckNone();
	else mainAdapter->CheckAll();
}
void CMainDlg::OnAlbumDeleteSelection()
{
	if (mainAdapter->GetCheckItemCount() == 0) ShowBigTip(L"请选择你需要删除的图片");
	else {
		auto* items = mainAdapter->GetCheckItems();
		for (auto it = items->begin(); it != items->end(); it++) {
			delete (*it);
			mainAlbumItems.erase(it);
		}
	}
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
	if (isLoading)  return;

	int index = mainAdapter->GetItemIndex(currentItem);
	if (index < 0) index = 0;
	if (index < (int)mainAlbumItems.size() - 1) 
		pLstViewMain->SetSel(index + 1, true);
	else ShowBigTip(L"已经是最后一张图片了");
}
void CMainDlg::OnPrev()
{
	if (isLoading)  return ;

	int index = mainAdapter->GetItemIndex(currentItem);
	if (index < 0) index = 0;
	if (index >= 1)
		pLstViewMain->SetSel(index - 1, true);
	else ShowBigTip(L"已经是第一张图片了");
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
bool CMainDlg::OnMainSelectionItemChanged(EventArgs* e)
{
	if (isLoading)  return true;

	EventLVSelChanged* e2 = sobj_cast<EventLVSelChanged>(e);
	int index = e2->iNewSel;
	CMainAlbumItem* item = mainAlbumItems[index];
	if (item != currentItem) {
		currentItem = item;
		gameRenderer->SetOpenFilePath(item->FilePath.c_str());
		gameRenderer->MarkShouldOpenFile();
	}
	return true;
}

//打开子窗口
//*********************************

void CMainDlg::OnSettings() {
	CSettingsDlg settingsDlg;
	settingsDlg.Create(m_hWnd);
	settingsDlg.SendMessage(WM_INITDIALOG);
	settingsDlg.CenterWindow(settingsDlg.m_hWnd);
	settingsDlg.DoModal(m_hWnd);
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
	SystemHelper::Open(L"https://www.imengyu.top/services/feedback?appname=720App");
}

//回调处理
//*********************************

void CMainDlg::OnThreadMessage(int handlerId)
{
	if (uiThreadMessageHandler != nullptr && handlerId == 2)
		uiThreadMessageHandler->RunNext();
}
void CMainDlg::OnGameFileStatusChanged(bool isOpen, int status) {

	isFileOpened = isOpen;
	if (isOpen) {

		LPCWSTR currentFileInfoTitle = gameRenderer->GetCurrentFileInfoTitle();
		SNativeWnd::SetWindowText(currentFileInfoTitle);
		pTextTitle->SetWindowText(currentFileInfoTitle);

		pWindowWelecome->SetVisible(false, true);
		pWindowMain->SetVisible(true, true);
		pBtnFullscreenMenu->SetVisible(true, true);
		view->SetVisible(true);

		currentItem = mainAlbumItems[pLstViewMain->GetSel()];
	}
	else {
		currentItem = nullptr;
		if (status == GAME_FILE_CLOSE_BUT_WILL_OPEN_NEXT) {
			SNativeWnd::SetWindowText(strTitleLoading);
			pTextTitle->SetWindowText(strTitleLoading);

			pWindowMain->SetVisible(true, true);
			pWindowWelecome->SetVisible(false, true);
			pWindowError->SetVisible(false, true);
		}
		else if (status == GAME_FILE_OPEN_FAILED) {

			LPCWSTR currentFileInfoTitle = gameRenderer->GetCurrentFileInfoTitle();
			SNativeWnd::SetWindowText(currentFileInfoTitle);
			pTextTitle->SetWindowText(currentFileInfoTitle);

			pWindowMain->SetVisible(true, true);
			pWindowWelecome->SetVisible(false, true);
			pWindowError->SetVisible(true, true);
			pTextFileError->SetWindowText(gameRenderer->GetFileLastError());
		}
		else {
			pTextTitle->SetWindowText(strTitle);
			SNativeWnd::SetWindowText(strTitle);

			if (isShowImageInfo) {
				isShowImageInfo = false;
				pWindowImageInfo->SetVisible(isShowImageInfo, true);
			}

			pWindowWelecome->SetVisible(true, true);
			pWindowMain->SetVisible(false, true);
			pBtnFullscreenMenu->SetVisible(false, true);
			view->SetVisible(false);
		}
	}

	UpdateToolButtonState();
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
	case GAME_EVENT_LOADING_STATUS: {
		thisPtr->isLoading = (bool)param;
		thisPtr->UpdateLoadingState();
		thisPtr->UpdateToolButtonState();
		break;
	}
	case GAME_EVENT_IMAGE_INFO_LOADED: {
		thisPtr->DoLoadImageInfo(1);
		break;
	}
	case GAME_EVENT_IMAGE_INFO_LOADED2: {
		thisPtr->DoLoadImageInfo(2);
		break;
	}
	default:
		break;
	}
}

//图库操作
//*********************************

bool CMainDlg::IsInAlbumList(const wchar_t* path) {
	for (auto it = mainAlbumItems.begin(); it != mainAlbumItems.end(); it++) {
		if ((*it)->FilePath == path)
			return true;
	}
	return false;
}
bool CMainDlg::AlbumListAdd(const wchar_t* path, CMainAlbumItem** outItem) {
	if (!IsInAlbumList(path)) {
		wchar_t* fileName = Path_GetFileName(path);

		CMainAlbumItem *newItem = new CMainAlbumItem(path, fileName);
		newItem->Type = 0;
		newItem->Checked = false;
		newItem->ThumbnailPath = L"";
		mainAlbumItems.push_back(newItem);

		if (outItem)
			*outItem = newItem;

		AppFree(fileName);
		return true;
	}	
	return false;
}
void CMainDlg::AlbumCheckItemChanged(void* data, int count)
{
	auto* _this = (CMainDlg*)data;
	if (count == 0)
		_this->pTextAlbumSelectTitle->SetWindowText(L"请选择照片");
	else {
		int c = count;
		_this->pTextAlbumSelectTitle->SetWindowText(FormatString(L"已选择 %d 张图像", c).c_str());
	}
}
void CMainDlg::LoadAlbumList() {
	std::wstring path = AppGetAppInstance()->GetCurrentDir();
	path += L"\\data\\album.xml";

	pugi::xml_document doc;
	doc.load_file(path.c_str());

	pugi::xml_node root = doc.child(L"root");
	//遍历 root
	for (pugi::xml_node item = root.first_child(); item != nullptr; item = item.next_sibling())
	{
		auto pathAttr = item.attribute(L"path");
		if (!pathAttr.empty())
			AlbumListAdd(pathAttr.as_string());
	}

	mainAdapter->notifyDataSetChanged();
}
void CMainDlg::SaveAlbumList() {
	std::wstring path = AppGetAppInstance()->GetCurrentDir();
	path += L"\\data\\album.xml";
	pugi::xml_document doc;
	pugi::xml_node root = doc.append_child(L"root");

	for (auto it = mainAlbumItems.begin(); it != mainAlbumItems.end(); it++) {
		pugi::xml_node item = root.append_child(L"item");
		pugi::xml_attribute pathAttr = item.append_attribute(L"path");
		pathAttr.set_value((*it)->FilePath.c_str());
	}

	doc.save_file(path.c_str());
}
void CMainDlg::ClearAlbumList() {
	for (auto it = mainAlbumItems.begin(); it != mainAlbumItems.end(); it++) 
		delete (* it);
	mainAlbumItems.clear();
}




