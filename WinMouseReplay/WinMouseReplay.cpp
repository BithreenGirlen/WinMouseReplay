// WinMouseReplay.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "main_window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
    int iRet = 0;
    CMainWindow* pWindow = new CMainWindow();
    if (pWindow != nullptr)
    {
        bool bRet = pWindow->Create(hInstance);
        if (bRet)
        {
            ShowWindow(pWindow->GetHwnd(), nCmdShow);
            iRet = pWindow->MesaageLoop();
        }
        delete pWindow;
    }

    return iRet;
}
