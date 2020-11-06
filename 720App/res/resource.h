//stamp:075ad1397d9963dc
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#pragma once
#include <res.mgr/snamedvalue.h>
#define ROBJ_IN_CPP \
namespace SOUI \
{\
    const _R R;\
    const _UIRES UIRES;\
}
namespace SOUI
{
	class _UIRES{
		public:
		class _UIDEF{
			public:
			_UIDEF(){
				XML_INIT = _T("UIDEF:XML_INIT");
			}
			const TCHAR * XML_INIT;
		}UIDEF;
		class _LAYOUT{
			public:
			_LAYOUT(){
				XML_MAINWND = _T("LAYOUT:XML_MAINWND");
			}
			const TCHAR * XML_MAINWND;
		}LAYOUT;
		class _values{
			public:
			_values(){
				string = _T("values:string");
				color = _T("values:color");
				skin = _T("values:skin");
			}
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
		}values;
		class _IMG{
			public:
			_IMG(){
				btn_close = _T("IMG:btn_close");
				btn_delete = _T("IMG:btn_delete");
				btn_fullscreen = _T("IMG:btn_fullscreen");
				btn_left = _T("IMG:btn_left");
				btn_max = _T("IMG:btn_max");
				btn_menu = _T("IMG:btn_menu");
				btn_min = _T("IMG:btn_min");
				btn_right = _T("IMG:btn_right");
				btn_restore = _T("IMG:btn_restore");
				btn_scale_100 = _T("IMG:btn_scale_100");
				btn_scale_org = _T("IMG:btn_scale_org");
				btn_tools = _T("IMG:btn_tools");
				btn_zoom_in = _T("IMG:btn_zoom_in");
				btn_zoom_out = _T("IMG:btn_zoom_out");
				btn_open = _T("IMG:btn_open");
				btn_info = _T("IMG:btn_info");
				window_bkgnd = _T("IMG:window_bkgnd");
			}
			const TCHAR * btn_close;
			const TCHAR * btn_delete;
			const TCHAR * btn_fullscreen;
			const TCHAR * btn_left;
			const TCHAR * btn_max;
			const TCHAR * btn_menu;
			const TCHAR * btn_min;
			const TCHAR * btn_right;
			const TCHAR * btn_restore;
			const TCHAR * btn_scale_100;
			const TCHAR * btn_scale_org;
			const TCHAR * btn_tools;
			const TCHAR * btn_zoom_in;
			const TCHAR * btn_zoom_out;
			const TCHAR * btn_open;
			const TCHAR * btn_info;
			const TCHAR * window_bkgnd;
		}IMG;
		class _ICON{
			public:
			_ICON(){
				ICON_LOGO = _T("ICON:ICON_LOGO");
			}
			const TCHAR * ICON_LOGO;
		}ICON;
	};
	const SNamedID::NAMEDVALUE namedXmlID[]={
		{L"_name_start",65535},
		{L"btn_album",65550},
		{L"btn_close",65536},
		{L"btn_delete",65549},
		{L"btn_info",65548},
		{L"btn_left_img",65545},
		{L"btn_max",65537},
		{L"btn_menu",65540},
		{L"btn_min",65539},
		{L"btn_open",65547},
		{L"btn_restore",65538},
		{L"btn_right_img",65546},
		{L"btn_scale_100",65541},
		{L"btn_scale_org",65542},
		{L"btn_zoom_in",65544},
		{L"btn_zoom_out",65543}		};
	class _R{
	public:
		class _name{
		public:
		_name(){
			_name_start = namedXmlID[0].strName;
			btn_album = namedXmlID[1].strName;
			btn_close = namedXmlID[2].strName;
			btn_delete = namedXmlID[3].strName;
			btn_info = namedXmlID[4].strName;
			btn_left_img = namedXmlID[5].strName;
			btn_max = namedXmlID[6].strName;
			btn_menu = namedXmlID[7].strName;
			btn_min = namedXmlID[8].strName;
			btn_open = namedXmlID[9].strName;
			btn_restore = namedXmlID[10].strName;
			btn_right_img = namedXmlID[11].strName;
			btn_scale_100 = namedXmlID[12].strName;
			btn_scale_org = namedXmlID[13].strName;
			btn_zoom_in = namedXmlID[14].strName;
			btn_zoom_out = namedXmlID[15].strName;
		}
		 const wchar_t * _name_start;
		 const wchar_t * btn_album;
		 const wchar_t * btn_close;
		 const wchar_t * btn_delete;
		 const wchar_t * btn_info;
		 const wchar_t * btn_left_img;
		 const wchar_t * btn_max;
		 const wchar_t * btn_menu;
		 const wchar_t * btn_min;
		 const wchar_t * btn_open;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_right_img;
		 const wchar_t * btn_scale_100;
		 const wchar_t * btn_scale_org;
		 const wchar_t * btn_zoom_in;
		 const wchar_t * btn_zoom_out;
		}name;

		class _id{
		public:
		const static int _name_start	=	65535;
		const static int btn_album	=	65550;
		const static int btn_close	=	65536;
		const static int btn_delete	=	65549;
		const static int btn_info	=	65548;
		const static int btn_left_img	=	65545;
		const static int btn_max	=	65537;
		const static int btn_menu	=	65540;
		const static int btn_min	=	65539;
		const static int btn_open	=	65547;
		const static int btn_restore	=	65538;
		const static int btn_right_img	=	65546;
		const static int btn_scale_100	=	65541;
		const static int btn_scale_org	=	65542;
		const static int btn_zoom_in	=	65544;
		const static int btn_zoom_out	=	65543;
		}id;

		class _string{
		public:
		const static int close	=	0;
		const static int fullscreen	=	1;
		const static int maximize	=	2;
		const static int menu	=	3;
		const static int minimize	=	4;
		const static int restore	=	5;
		const static int title	=	6;
		const static int ver	=	7;
		}string;

		class _color{
		public:
		const static int blue	=	0;
		const static int gray	=	1;
		const static int green	=	2;
		const static int red	=	3;
		const static int white	=	4;
		}color;

	};

#ifdef R_IN_CPP
	 extern const _R R;
	 extern const _UIRES UIRES;
#else
	 extern const __declspec(selectany) _R & R = _R();
	 extern const __declspec(selectany) _UIRES & UIRES = _UIRES();
#endif//R_IN_CPP
}
