// magicbox2.cpp : main source file for magicbox2.exe
//

#include "stdafx.h"
#include <com-cfg.h> 
#include "MainDlg.h"
#include "SouiRealWndHandler.h"

#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <share.h>
#include <sys/stat.h>

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	CScintillaModule sciMod;
	if(!sciMod) return -1;

	HRESULT hRes = CoInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));
 
	TCHAR szCurrentDir[MAX_PATH+1]; memset( szCurrentDir, 0, sizeof(szCurrentDir) );
	GetModuleFileName( NULL, szCurrentDir, MAX_PATH );
	LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
    _tcscpy(lpInsertPos,_T("\\..\\UIEditor"));
    SetCurrentDirectory(szCurrentDir);

    int nRet = 0; 

    SComMgr *pComMgr = new SComMgr;

    {
        CAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory; //图片解码器，由imagedecoder-wid.dll模块提供
        CAutoRefPtr<IRenderFactory> pRenderFactory;         //UI渲染模块，由render-gdi.dll或者render-skia.dll提供
        BOOL bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("render_gdi"));
        bLoaded=pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));
        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

        CAutoRefPtr<IResProvider>   pResProvider;
#ifdef _DEBUG//从文件加载
        CreateResProvider(RES_FILE,(IObjRef**)&pResProvider);
        if(!pResProvider->Init((LPARAM)_T("uires"),0))
        {
            SASSERT(0);
            return 1;
        }
#else //从EXE资源加载
        CreateResProvider(RES_PE,(IObjRef**)&pResProvider);
        pResProvider->Init((WPARAM)hInstance,0);
#endif

        SApplication theApp(pRenderFactory,hInstance);

        //将创建的IResProvider交给SApplication对象
        theApp.AddResProvider(pResProvider);

        //注册图片预览控件
        RegSkinViewClass(&theApp);
        theApp.RegisterWndFactory(TplSWindowFactory<SColorPicker>());

        BOOL bOK=theApp.Init(_T("IDR_DUI_INIT")); //初始化DUI系统,原来的系统初始化方式依然可以使用。


        CSouiRealWndHandler * pRealWndHandler = new CSouiRealWndHandler();
        theApp.SetRealWndHandler(pRealWndHandler);
        pRealWndHandler->Release();

        //加载系统资源
        HMODULE hSysResource=LoadLibrary(SYS_NAMED_RESOURCE);
        if(hSysResource)
        {
            CAutoRefPtr<IResProvider> sysSesProvider;
            CreateResProvider(RES_PE,(IObjRef**)&sysSesProvider);
            sysSesProvider->Init((WPARAM)hSysResource,0);
            theApp.LoadSystemNamedResource(sysSesProvider);
        }

        // BLOCK: Run application
        {
            CMainDlg dlgMain;  
            nRet = dlgMain.DoModal();  
        }
    }

    delete pComMgr;

	CoUninitialize();
	return nRet;
}
