// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "helper/SMenu.h"
#include "../controls.extend/FileHelper.h"
#include "../controls.extend/SChatEdit.h"
#include "../controls.extend/reole/richeditole.h"
#include "FormatMsgDlg.h"
#include <math.h>
#include <helper/SAdapterBase.h>

#pragma warning(disable:4192)

#ifdef _DEBUG
#import "..\bin\SoSmileyd.dll" named_guids
#else
#import "..\bin\SoSmiley.dll" named_guids
#endif

#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")

#include <shellapi.h>
class CTestDropTarget:public IDropTarget
{
public:
    CTestDropTarget()
    {
        nRef=0;
    }
    
    virtual ~CTestDropTarget(){}
    
    //////////////////////////////////////////////////////////////////////////
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        HRESULT hr=S_FALSE;
        if(riid==__uuidof(IUnknown))
            *ppvObject=(IUnknown*) this,hr=S_OK;
        else if(riid==__uuidof(IDropTarget))
            *ppvObject=(IDropTarget*)this,hr=S_OK;
        if(SUCCEEDED(hr)) AddRef();
        return hr;

    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void){return ++nRef;}

    virtual ULONG STDMETHODCALLTYPE Release( void) { 
        ULONG uRet= -- nRef;
        if(uRet==0) delete this;
        return uRet;
    }

    //////////////////////////////////////////////////////////////////////////
    // IDropTarget

    virtual HRESULT STDMETHODCALLTYPE DragEnter( 
        /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE DragOver( 
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE DragLeave( void)
    {
        return S_OK;
    }


protected:
    int nRef;
};

class CTestDropTarget1 : public CTestDropTarget
{
protected:
    SWindow *m_pEdit;
public:
    CTestDropTarget1(SWindow *pEdit):m_pEdit(pEdit)
    {
        if(m_pEdit) m_pEdit->AddRef();
    }
    ~CTestDropTarget1()
    {
        if(m_pEdit) m_pEdit->Release();
    }
public:
    virtual HRESULT STDMETHODCALLTYPE Drop( 
        /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        FORMATETC format =
        {
            CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
        };
        STGMEDIUM medium;
        if(FAILED(pDataObj->GetData(&format, &medium)))
        {
            return S_FALSE;
        }

        HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

        if(!hdrop)
        {
            return S_FALSE;
        }

        bool success = false;
        TCHAR filename[MAX_PATH];
        success=!!DragQueryFile(hdrop, 0, filename, MAX_PATH);
        DragFinish(hdrop);
        GlobalUnlock(medium.hGlobal);

        if(success && m_pEdit)
        {
            m_pEdit->SetWindowText(filename);
        }

        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }
};



class CTestAdapterFix : public SAdapterBase
{
public:
    CTestAdapterFix()
    {

    }
    virtual int getCount()
    {
        return 50000;
    }   

    virtual void getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate)
    {
        if(pItem->GetChildrenCount()==0)
        {
            pItem->InitFromXml(xmlTemplate);
        }
        SAnimateImgWnd *pAni = pItem->FindChildByName2<SAnimateImgWnd>(L"ani_test");

        SButton *pBtn = pItem->FindChildByName2<SButton>(L"btn_test");
        pBtn->SetWindowText(SStringW().Format(L"button %d",position));
        pBtn->SetUserData(position);
        pBtn->GetEventSet()->subscribeEvent(EVT_CMD,Subscriber(&CTestAdapterFix::OnButtonClick,this));
    }

    bool OnButtonClick(EventArgs *pEvt)
    {
        SButton *pBtn = sobj_cast<SButton>(pEvt->sender);
        int iItem = pBtn->GetUserData();
        SMessageBox(NULL,SStringT().Format(_T("button of %d item was clicked"),iItem),_T("haha"),MB_OK);
        return true;
    }
};


class CTestAdapterFlex : public SAdapterBase
{
public:
    CTestAdapterFlex()
    {

    }
    virtual int getCount()
    {
        return 12340;
    }   

    virtual int getViewTypeCount(){return 2;}
    
    virtual int getItemViewType(int position)
    {
        return position%2;
    }
    
    virtual void getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate)
    {
        const TCHAR * szSent[]={
            _T("Five score years ago, a great American, in whose symbolic shadow we stand today, signed the Emancipation Proclamation. This momentous decree came as a great beacon light of hope to millions of Negro slaves who had been seared in the flames of withering injustice. It came as a joyous daybreak to end the long night of bad captivity."),
            _T("But one hundred years later, the Negro still is not free. One hundred years later, the life of the Negro is still sadly crippled by the manacles of segregation and the chains of discrimination. One hundred years later, the Negro lives on a lonely island of poverty in the midst of a vast ocean of material prosperity. One hundred years later, the Negro is still languished in the corners of American society and finds himself an exile in his own land. So we’ve come here today to dramatize a shameful condition."),
            _T("I am not unmindful that some of you have come here out of great trials and tribulations. Some of you have come fresh from narrow jail cells. Some of you have come from areas where your quest for freedom left you battered by the storms of persecution and staggered by the winds of police brutality. You have been the veterans of creative suffering. Continue to work with the faith that unearned suffering is redemptive."),
            _T("Go back to Mississippi, go back to Alabama, go back to South Carolina, go back to Georgia, go back to Louisiana, go back to the slums and ghettos of our northern cities, knowing that somehow this situation can and will be changed. Let us not wallow in the valley of despair.")
        };
        
        if(pItem->GetChildrenCount()==0)
        {
            const wchar_t *pTemplate = position%2==0?L"itemOdd":L"itemEven";
            pItem->InitFromXml(xmlTemplate.child(pTemplate));
        }
        if(position%2==0)
        {
            int iSent = (position/2)%4;
            SStatic *pTxtDream = pItem->FindChildByName2<SStatic>(L"txt_dream");
            SASSERT(pTxtDream);
            pTxtDream->SetWindowText(szSent[iSent]);
        }
        
        SButton *pBtn = pItem->FindChildByName2<SButton>(L"btn_test");
        pBtn->SetWindowText(SStringW().Format(L"button %d",position));
        pBtn->SetUserData(position);
        pBtn->GetEventSet()->subscribeEvent(EVT_CMD,Subscriber(&CTestAdapterFlex::OnButtonClick,this));
    }

    bool OnButtonClick(EventArgs *pEvt)
    {
        SButton *pBtn = sobj_cast<SButton>(pEvt->sender);
        int iItem = pBtn->GetUserData();
        SMessageBox(NULL,SStringT().Format(_T("button of %d item was clicked"),iItem),_T("haha"),MB_OK);
        return true;
    }
    
};

class CTestMcAdapterFix : public SMcAdapterBase
{
public:
struct SOFTINFO
{
    wchar_t * pszSkinName;
    wchar_t * pszName;
    wchar_t * pszDesc;
    float     fScore;
    DWORD     dwSize;
    wchar_t * pszInstallTime;
    wchar_t * pszUseTime;
};

static SOFTINFO s_info[];

public:
    CTestMcAdapterFix()
    {

    }

    virtual int getCount()
    {
        return 12340;
    }   

    SStringT getSizeText(DWORD dwSize)
    {
        int num1=dwSize/(1<<20);
        dwSize -= num1 *(1<<20);
        int num2 = dwSize*100/(1<<20);
        return SStringT().Format(_T("%d.%02dM"),num1,num2);
    }
    
    virtual void getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate)
    {
        if(pItem->GetChildrenCount()==0)
        {
            pItem->InitFromXml(xmlTemplate);
        }
        int dataSize = 7;
        SOFTINFO *psi = s_info+position%dataSize;
        pItem->FindChildByName(L"img_icon")->SetAttribute(L"skin",psi->pszSkinName);
        pItem->FindChildByName(L"txt_name")->SetWindowText(S_CW2T(psi->pszName));
        pItem->FindChildByName(L"txt_desc")->SetWindowText(S_CW2T(psi->pszDesc));
        pItem->FindChildByName(L"txt_score")->SetWindowText(SStringT().Format(_T("%1.2f 分"),psi->fScore));
        pItem->FindChildByName(L"txt_installtime")->SetWindowText(S_CW2T(psi->pszInstallTime));
        pItem->FindChildByName(L"txt_usetime")->SetWindowText(S_CW2T(psi->pszUseTime));
        pItem->FindChildByName(L"txt_size")->SetWindowText(getSizeText(psi->dwSize));
        pItem->FindChildByName2<SRatingBar>(L"rating_score")->SetValue(psi->fScore/2);
        pItem->FindChildByName(L"txt_index")->SetWindowText(SStringT().Format(_T("第%d行"),position));
        
        SButton *pBtnUninstall = pItem->FindChildByName2<SButton>(L"btn_uninstall");
        pBtnUninstall->SetUserData(position);
        pBtnUninstall->GetEventSet()->subscribeEvent(EVT_CMD,Subscriber(&CTestMcAdapterFix::OnButtonClick,this));
    }

    bool OnButtonClick(EventArgs *pEvt)
    {
        SButton *pBtn = sobj_cast<SButton>(pEvt->sender);
        int iItem = pBtn->GetUserData();
        SMessageBox(NULL,SStringT().Format(_T("button of %d item was clicked"),iItem),_T("uninstall"),MB_OK);
        return true;
    }

    SStringW GetColumnName(int iCol) const{
        return SStringW().Format(L"col%d",iCol+1);
    }
    
    struct SORTCTX
    {
        int iCol;
        SHDSORTFLAG stFlag;
    };
    
    bool OnSort(int iCol,SHDSORTFLAG * stFlags,int nCols)
    {
        if(iCol==5) //最后一列“操作”不支持排序
            return false;
        
        SHDSORTFLAG stFlag = stFlags[iCol];
        switch(stFlag)
        {
            case ST_NULL:stFlag = ST_UP;break;
            case ST_DOWN:stFlag = ST_UP;break;
            case ST_UP:stFlag = ST_DOWN;break;
        }
        for(int i=0;i<nCols;i++)
        {
            stFlags[i]=ST_NULL;
        }
        stFlags[iCol]=stFlag;
        
        SORTCTX ctx={iCol,stFlag};
        qsort_s(s_info,7,sizeof(SOFTINFO),SortCmp,&ctx);
        return true;
    }
    
    static int __cdecl SortCmp(void *context,const void * p1,const void * p2)
    {
        SORTCTX *pctx = (SORTCTX*)context;
        const SOFTINFO *pSI1=(const SOFTINFO*)p1;
        const SOFTINFO *pSI2=(const SOFTINFO*)p2;
        int nRet =0;
        switch(pctx->iCol)
        {
            case 0://name
                nRet = wcscmp(pSI1->pszName,pSI2->pszName);
                break;
            case 1://score
                {
                    float fCmp = (pSI1->fScore - pSI2->fScore);
                    if(fabs(fCmp)<0.0000001) nRet = 0;
                    else if(fCmp>0.0f) nRet = 1;
                    else nRet = -1;
                }
                break;
            case 2://size
                nRet = (int)(pSI1->dwSize - pSI2->dwSize);
                break;
            case 3://install time
                nRet = wcscmp(pSI1->pszInstallTime,pSI2->pszInstallTime);
                break;
            case 4://user time
                nRet = wcscmp(pSI1->pszUseTime,pSI2->pszUseTime);
                break;

        }
        if(pctx->stFlag == ST_UP)
            nRet = -nRet;
        return nRet;
    }
};

CTestMcAdapterFix::SOFTINFO CTestMcAdapterFix::s_info[] =
{
    {
        L"skin_icon1",
        L"鲁大师",
        L"鲁大师是一款专业的硬件检测，驱动安装工具",
        5.4f,
        15*(1<<20),
        L"2015-8-5",
        L"今天"
    },
    {
        L"skin_icon2",
        L"PhotoShop",
        L"强大的图片处理工具",
        9.0f,
        150*(1<<20),
        L"2015-8-5",
        L"今天"
    },
    {
        L"skin_icon3",
        L"QQ7.0",
        L"腾讯公司出品的即时聊天工具",
        8.0f,
        40*(1<<20),
        L"2015-8-5",
        L"今天"
    },
    {
        L"skin_icon4",
        L"Visual Studio 2008",
        L"Microsoft公司的程序开发套件",
        9.0f,
        40*(1<<20),
        L"2015-8-5",
        L"今天"
    },
    {
        L"skin_icon5",
        L"YY8",
        L"YY语音",
        9.0f,
        20*(1<<20),
        L"2015-8-5",
        L"今天"
    },
    {
        L"skin_icon6",
        L"火狐浏览器",
        L"速度最快的浏览器",
        8.5f,
        35*(1<<20),
        L"2015-8-5",
        L"今天"
    },
    {
        L"skin_icon7",
        L"迅雷",
        L"迅雷下载软件",
        7.3f,
        17*(1<<20),
        L"2015-8-5",
        L"今天"
    }
};


int CMainDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
//     MARGINS mar = {5,5,30,5};
//     DwmExtendFrameIntoClientArea ( m_hWnd, &mar );//打开这里可以启用Aero效果
	SetMsgHandled(FALSE);
	return 0;
}

void CMainDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if(bShow)
	{
 		AnimateHostWindow(200,AW_CENTER);
	}
}

struct student{
    TCHAR szName[100];
    TCHAR szSex[10];
    int age;
    int score;
};

//init listctrl
void CMainDlg::InitListCtrl()
{
    //找到列表控件
    SListCtrl *pList=FindChildByName2<SListCtrl>(L"lc_test");
    if(pList)
    {
        //列表控件的唯一子控件即为表头控件
        SWindow *pHeader=pList->GetWindow(GSW_FIRSTCHILD);
        //向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
        pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,Subscriber(&CMainDlg::OnListHeaderClick,this));

        TCHAR szSex[][5]={_T("男"),_T("女"),_T("人妖")};
        for(int i=0;i<100;i++)
        {
            student *pst=new student;
            _stprintf(pst->szName,_T("学生_%d"),i+1);
            _tcscpy(pst->szSex,szSex[rand()%3]);
            pst->age=rand()%30;
            pst->score=rand()%60+40;

            int iItem=pList->InsertItem(i,pst->szName);
            pList->SetItemData(iItem,(DWORD)pst);
            pList->SetSubItemText(iItem,1,pst->szSex);
            TCHAR szBuf[10];
            _stprintf(szBuf,_T("%d"),pst->age);
            pList->SetSubItemText(iItem,2,szBuf);
            _stprintf(szBuf,_T("%d"),pst->score);
            pList->SetSubItemText(iItem,3,szBuf);
        }
    }
}

int funCmpare(void* pCtx,const void *p1,const void *p2)
{
    int iCol=*(int*)pCtx;

    const DXLVITEM *plv1=(const DXLVITEM*)p1;
    const DXLVITEM *plv2=(const DXLVITEM*)p2;

    const student *pst1=(const student *)plv1->dwData;
    const student *pst2=(const student *)plv2->dwData;

    switch(iCol)
    {
    case 0://name
        return _tcscmp(pst1->szName,pst2->szName);
    case 1://sex
        return _tcscmp(pst1->szSex,pst2->szSex);
    case 2://age
        return pst1->age-pst2->age;
    case 3://score
        return pst1->score-pst2->score;
    default:
        return 0;
    }
}

//表头点击事件处理函数
bool CMainDlg::OnListHeaderClick(EventArgs *pEvtBase)
{
    //事件对象强制转换
    EventHeaderClick *pEvt =(EventHeaderClick*)pEvtBase;
    SHeaderCtrl *pHeader=(SHeaderCtrl*)pEvt->sender;
    //从表头控件获得列表控件对象
    SListCtrl *pList= (SListCtrl*)pHeader->GetParent();
    //列表数据排序
    SHDITEM hditem;
    hditem.mask=SHDI_ORDER;
    pHeader->GetItem(pEvt->iItem,&hditem);
    pList->SortItems(funCmpare,&hditem.iOrder);
    return true;
}

void CMainDlg::OnDestory()
{
    SListCtrl *pList=FindChildByName2<SListCtrl>(L"lc_test");
    if(pList)
    {
        for(int i=0;i<pList->GetItemCount();i++)
        {
            student *pst=(student*) pList->GetItemData(i);
            delete pst;
        }
    }
    SetMsgHandled(FALSE); 
}


class CSmileySource2 : public CSmileySource
{
public:
    CSmileySource2(){}

protected:
    //获对ID对应的图片路径
    virtual SStringW ImageID2Path(UINT nID)
    {
        return SStringW().Format(L"./gif/%d.gif",nID);
    }
};

//Richedit中插入表情使用的回调函数。
ISmileySource * CreateSource2()
{
    return  new CSmileySource2;
}

LRESULT CMainDlg::OnInitDialog( HWND hWnd, LPARAM lParam )
{
    m_bLayoutInited=TRUE;
    InitListCtrl();
        
    //演示在SOUI中的拖放
    SWindow *pEdit1 = FindChildByName(L"edit_drop_top");
    SWindow *pEdit2 = FindChildByName(L"edit_drop_bottom");
    if(pEdit1 && pEdit2)
    {
        HRESULT hr=::RegisterDragDrop(m_hWnd,GetDropTarget());
        RegisterDragDrop(pEdit1->GetSwnd(),new CTestDropTarget1(pEdit1));
        RegisterDragDrop(pEdit2->GetSwnd(),new CTestDropTarget1(pEdit2));
    }
    
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        SetSRicheditOleCallback(pEdit,CreateSource2);
        pEdit->SetAttribute(L"rtf",L"rtf:rtf_test");
    }

    //演示如何响应Edit的EN_CHANGE事件
    SEdit *pEditUrl = FindChildByName2<SEdit>(L"edit_url");
    if(pEditUrl)
    {
        pEditUrl->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
    }
    
    //演示SetWindowRgn用法
    SWindow *pWndRgn = FindChildByName(L"wnd_rgn");
    if(pWndRgn)
    {
        CRect rc=pWndRgn->GetWindowRect();
        rc.MoveToXY(0,0);//注意：SWindow将窗口的左上角定义为Rgn的原点。
        HRGN hRgn =::CreateEllipticRgnIndirect(&rc);

        CAutoRefPtr<IRegion> pRgn;
        GETRENDERFACTORY->CreateRegion(&pRgn);
        pRgn->SetRgn(hRgn);
        pWndRgn->SetWindowRgn(pRgn,TRUE);

        DeleteObject(hRgn);
    }
    
    //行高固定的列表
    SListView *pLstViewFix = FindChildByName2<SListView>("lv_test_fix");
    if(pLstViewFix)
    {
        IAdapter *pAdapter = new CTestAdapterFix;
        pLstViewFix->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    //行高可变的列表
    SListView *pLstViewFlex = FindChildByName2<SListView>("lv_test_flex");
    if(pLstViewFlex)
    {
        IAdapter *pAdapter = new CTestAdapterFlex;
        pLstViewFlex->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    //多列listview
    SMCListView * pMcListView = FindChildByName2<SMCListView>("mclv_test");
    if(pMcListView)
    {
        IMcAdapter *pAdapter = new CTestMcAdapterFix;
        pMcListView->SetAdapter(pAdapter);
        pAdapter->Release();
    }
    return 0;
}

void CMainDlg::OnBtnWebkitGo()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        SEdit *pEdit=FindChildByName2<SEdit>(L"edit_url");
        SStringT strUrl=pEdit->GetWindowText();
        pWebkit->SetAttribute(L"url",S_CT2W(strUrl),FALSE);
    }
}

void CMainDlg::OnBtnWebkitBackward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goBack();
    }
}

void CMainDlg::OnBtnWebkitForeward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goForward();
    }
}

void CMainDlg::OnBtnWebkitRefresh()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->reload();
    }
}

void CMainDlg::OnBtnSelectGIF()
{
    SGifPlayer *pGifPlayer = FindChildByName2<SGifPlayer>(L"giftest");
    if(pGifPlayer)
    {
        CFileDialogEx openDlg(TRUE,_T("gif"),0,6,_T("gif files(*.gif)\0*.gif\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
            pGifPlayer->PlayGifFile(openDlg.m_szFileName);
    }
}

void CMainDlg::OnBtnMenu()
{
    SMenu menu;
    menu.LoadMenu(_T("menu_test"),_T("LAYOUT"));
    POINT pt;
    GetCursorPos(&pt);
    menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

//演示如何响应菜单事件
void CMainDlg::OnCommand( UINT uNotifyCode, int nID, HWND wndCtl )
{
    if(uNotifyCode==0)
    {
        if(nID==6)
        {//nID==6对应menu_test定义的菜单的exit项。
            PostMessage(WM_CLOSE);
        }else if(nID==54)
        {//about SOUI
            STabCtrl *pTabCtrl = FindChildByName2<STabCtrl>(L"tab_main");
            if(pTabCtrl) pTabCtrl->SetCurSel(_T("about"));
        }
    }
}

void CMainDlg::OnBtnHideTest()
{
    SWindow * pBtn = FindChildByName(L"btn_display");
    if(pBtn) pBtn->SetVisible(!pBtn->IsVisible(TRUE),TRUE);
}


void CMainDlg::OnBtnInsertGif2RE()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(TRUE,_T("gif"),0,6,_T("gif files(*.gif)\0*.gif\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
            ISmileySource* pSource = new CSmileySource2;
            HRESULT hr=pSource->LoadFromFile(S_CT2W(openDlg.m_szFileName));
            if(SUCCEEDED(hr))
            {
                SComPtr<ISoSmileyCtrl> pSmiley;
                hr=::CoCreateInstance(SoSmiley::CLSID_CSoSmileyCtrl,NULL,CLSCTX_INPROC,__uuidof(SoSmiley::ISoSmileyCtrl),(LPVOID*)&pSmiley); 
                if(SUCCEEDED(hr))
                {
                    pSmiley->SetSource(pSource);
                    SComPtr<IRichEditOle> ole;
                    pEdit->SSendMessage(EM_GETOLEINTERFACE,0,(LPARAM)&ole);
                    pSmiley->Insert2Richedit((DWORD_PTR)(void*)ole);
                }else
                {
                    UINT uRet = SMessageBox(m_hWnd,_T("可能是因为没有向系统注册表情COM模块。\\n现在注册吗?"),_T("创建表情OLE对象失败"),MB_YESNO|MB_ICONSTOP);
                    if(uRet == IDYES)
                    {
                        HMODULE hMod = LoadLibrary(_T("sosmiley.dll"));
                        if(hMod)
                        {
                            typedef HRESULT (STDAPICALLTYPE *DllRegisterServerPtr)();
                            DllRegisterServerPtr funRegDll = (DllRegisterServerPtr)GetProcAddress(hMod,"DllRegisterServer");
                            if(funRegDll)
                            {
                                HRESULT hr=funRegDll();
                                if(FAILED(hr))
                                {
                                    SMessageBox(m_hWnd,_T("请使用管理员权限运行模块注册程序"),_T("注册表情COM失败"),MB_OK|MB_ICONSTOP);
                                }else
                                {
                                    SMessageBox(m_hWnd,_T("请重试"),_T("注册成功"),MB_OK|MB_ICONINFORMATION);
                                }
                            }
                            FreeLibrary(hMod);
                        }else
                        {
                            SMessageBox(m_hWnd,_T("没有找到表情COM模块[sosmiley.dll]。\\n现在注册吗"),_T("错误"),MB_OK|MB_ICONSTOP);
                        }
                    }
                }
            }else
            {
                SMessageBox(m_hWnd,_T("加载表情失败"),_T("错误"),MB_OK|MB_ICONSTOP);
            }
            pSource->Release();
        }
    }
}

void CMainDlg::OnBtnAppendMsg()
{
    SChatEdit *pEdit = FindChildByName2<SChatEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFormatMsgDlg formatMsgDlg;
        if(formatMsgDlg.DoModal()==IDOK)
        {
            for(int i=0;i<formatMsgDlg.m_nRepeat;i++)
                pEdit->AppendFormatText(S_CT2W(SStringT().Format(_T("line:%d "),i) + formatMsgDlg.m_strMsg));
        }
    }
}

void CMainDlg::OnBtnMsgBox()
{
    SMessageBox(NULL,_T("this is a message box"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
    SMessageBox(NULL,_T("this message box includes two buttons"),_T("haha"),MB_YESNO|MB_ICONQUESTION);
    SMessageBox(NULL,_T("this message box includes three buttons"),NULL,MB_ABORTRETRYIGNORE);
}

#include <render-skia/Render-Skia2-i.h>

void CMainDlg::OnSkiaTest()
{
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,100,100);
    CAutoRefPtr<IRenderTarget_Skia2> pRTSkia2;
    HRESULT hr=pRT->QueryInterface(__uuidof(IRenderTarget_Skia2),(IObjRef**)&pRTSkia2);
    if(SUCCEEDED(hr))
    {
        CRect rcUp(0,0,100,50);
        CRect rcDown(0,50,100,100);
        pRT->FillSolidRect(&rcUp,RGBA(255,0,0,255));
        pRT->FillSolidRect(&rcDown,RGBA(0,255,0,255));
        
        SWindow *pCanvas = FindChildByName(L"skia_canvas");
        if(pCanvas)
        {
            IRenderTarget* pRTDst= pCanvas->GetRenderTarget(0,OLEDC_PAINTBKGND);
            CRect rcCanvas;
            pCanvas->GetWindowRect(&rcCanvas);
            
            CRect rcDst(rcCanvas.TopLeft(),CSize(100,100));
            pRTDst->BitBlt(&rcDst,pRT,0,0);
            
            pRTSkia2->Init(pRT);
            pRTSkia2->rotate(45.0f);
            pRT->FillSolidRect(&rcUp,RGBA(0,0,255,255));
            pRT->FillSolidRect(&rcDown,RGBA(0,255,255,255));
            
            
            rcDst.OffsetRect(100,0);
            pRTDst->BitBlt(&rcDst,pRT,0,0);
            
            pCanvas->ReleaseRenderTarget(pRTDst);
        }
    }else
    {
        SMessageBox(NULL,_T("当前使用的渲染引擎不是skia"),_T("错误"),MB_OK|MB_ICONSTOP);
    }
}

void CMainDlg::OnListBoxExEvent( EventArgs *pEvt )
{
    EventOfPanel *pEvtOfPanel = (EventOfPanel*)pEvt;
    if(pEvtOfPanel->pOrgEvt->GetID() == EventCmd::EventID
        && pEvtOfPanel->pOrgEvt->sender->IsClass(SButton::GetClassName()))
    {
        int iItem = pEvtOfPanel->pPanel->GetItemIndex();
        SStringT strMsg;
        strMsg.Format(_T("收到列表项:%d中的name为%s的窗口点击事件"),iItem,S_CW2T(pEvtOfPanel->pOrgEvt->nameFrom));
        SMessageBox(m_hWnd,strMsg,_T("EVENTOFPANEL"),MB_OK|MB_ICONEXCLAMATION);        
    }
}

void CMainDlg::OnTreeBoxEvent( EventArgs *pEvt )
{
    EventOfPanel *pEvtOfPanel = (EventOfPanel*)pEvt;
    if(pEvtOfPanel->pOrgEvt->GetID() == EventCmd::EventID
        && pEvtOfPanel->pOrgEvt->sender->IsClass(SButton::GetClassName()))
    {
        HSTREEITEM hItem = (HSTREEITEM)pEvtOfPanel->pPanel->GetItemIndex();
        SStringT strMsg;
        strMsg.Format(_T("收到treebox item:0x%08x中的name为%s的窗口点击事件"),hItem,S_CW2T(pEvtOfPanel->pOrgEvt->nameFrom));
        SMessageBox(m_hWnd,strMsg,_T("EVENTOFPANEL"),MB_OK|MB_ICONEXCLAMATION);        
    }

    if(pEvtOfPanel->pOrgEvt->GetID() >= EVT_ITEMPANEL_CLICK && pEvtOfPanel->pOrgEvt->GetID() <= EVT_ITEMPANEL_RCLICK)
    {
        HSTREEITEM hItem = (HSTREEITEM)pEvtOfPanel->pPanel->GetItemIndex();
        STRACE(_T("OnTreeBoxEvent: EVT_ITEMPANEL_X, itemid=0x%08x,evtid=%d"),hItem,pEvtOfPanel->pOrgEvt->GetID());
    }
}

void CMainDlg::OnTabPageRadioSwitch(EventArgs *pEvt)
{
    EventSwndStateChanged *pEvt2 = sobj_cast<EventSwndStateChanged>(pEvt);
    if(pEvt2->CheckState(WndState_Check) && (pEvt2->dwNewState & WndState_Check))
    {
        int id= pEvt->idFrom;
        STabCtrl *pTab =FindChildByName2<STabCtrl>(L"tab_radio2");
        if(pTab) pTab->SetCurSel(id-10000);
    }
}

void CMainDlg::OnBtnRtfSave()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(FALSE,_T("rtf"),_T("soui_richedit"),6,_T("rtf files(*.rtf)\0*.rtf\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
            pEdit->SaveRtf(openDlg.m_szFileName);
        }
    }
}

void CMainDlg::OnBtnRtfOpen()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(TRUE,_T("rtf"),0,6,_T("rtf files(*.rtf)\0*.rtf\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
            pEdit->LoadRtf(openDlg.m_szFileName);
        }
    }
}

void CMainDlg::OnTreeBoxQueryItemHeight( EventArgs * pEvt )
{
    EventTBQueryItemHeight *pEvtTbQueryItemHeight = (EventTBQueryItemHeight*)pEvt;
    STreeBox *pTreeBox = (STreeBox*)pEvt->sender;
    STreeItem *pItem = pTreeBox->GetItemPanel(pEvtTbQueryItemHeight->hItem);

    if(pItem->m_nLevel>0)
    {
        if(pEvtTbQueryItemHeight->dwState & WndState_Check)
            pEvtTbQueryItemHeight->nItemHeight = 40;
        else
            pEvtTbQueryItemHeight->nItemHeight = 30;
    }else
    {
        pEvtTbQueryItemHeight->nItemHeight = 50;
    }
}

void CMainDlg::OnChromeTabNew( EventArgs *pEvt )
{
    static int iPage = 0;
    EventChromeTabNew *pEvtTabNew = (EventChromeTabNew*)pEvt;

    SStringT strTitle = SStringT().Format(_T("新建窗口 %d"),++iPage);
    pEvtTabNew->pNewTab->SetWindowText(strTitle);
    pEvtTabNew->pNewTab->SetAttribute(L"tip",S_CT2W(strTitle));
}

//演示如何使用文件资源创建窗口
void CMainDlg::OnBtnFileWnd()
{
    //由于资源中使用了相对路径，需要将当前路径指定到资源所在位置
    SStringT strCurDir = SApplication::getSingleton().GetAppDir();
    strCurDir += _T("\\filewnd");
    SetCurrentDirectory(strCurDir);
    if(GetFileAttributes(_T("test.xml"))==INVALID_FILE_ATTRIBUTES)
    {
        SMessageBox(m_hWnd,_T("没有找到资源文件！"),_T("错误"),MB_OK|MB_ICONSTOP);
        return ;
    }
    SHostDialog fileDlg(_T("file:test.xml"));
    fileDlg.DoModal(m_hWnd);
}

//演示如何响应Edit的EN_CHANGE事件
void CMainDlg::OnUrlReNotify(EventArgs *pEvt)
{
    EventRENotify *pEvt2 = sobj_cast<EventRENotify>(pEvt);
    STRACE(_T("OnUrlReNotify,iNotify = %d"),pEvt2->iNotify);
    if(pEvt2->iNotify == EN_CHANGE)
    {
        STRACE(_T("OnUrlReNotify,iNotify = EN_CHANGE"));    
    }
}
