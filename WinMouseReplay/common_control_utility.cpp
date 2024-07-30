

#include "common_control_utility.h"

#include <CommCtrl.h>

namespace combo_box_utility
{
    /*番号整形*/
    std::string FormatInt(int iValue, int iLength)
    {
        std::string str;

        int iDigits = 0;
        int iNumber = iValue;
        if (iValue <= 0)
        {
            iDigits = 1;
            if (iValue < 0)
            {
                iValue = -iValue;
                str += '-';
            }
        }

        while (iNumber)
        {
            iNumber /= 10;
            ++iDigits;
        }

        int iToBeAdded = iLength - iDigits;
        for (int i = 0; i < iToBeAdded; ++i)
        {
            str += '0';
        }
        str += std::to_string(iValue);
        return str;
    }

    /*項目消去*/
    void ClearComboBox(HWND hComboBox)
    {
        ::SendMessage(hComboBox, CB_RESETCONTENT, 0, 0);
    }

    /*コンボボックス選択番号取得*/
    int GetComboBoxSelectedItemIndex(HWND hComboBox)
    {
        if (hComboBox != nullptr)
        {
            LRESULT lResult = ::SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
            return static_cast<int>(lResult);
        }
        return CB_ERR;
    }
}
// namespace combo_box_utility

/*選択中のコンボボックス文字列取得*/
std::string combo_box_utility::GetComboBoxText(HWND hComboBox)
{
    if (hComboBox != nullptr)
    {
        int iIndex = GetComboBoxSelectedItemIndex(hComboBox);
        if (iIndex != CB_ERR)
        {
            LRESULT lResult = ::SendMessageA(hComboBox, CB_GETLBTEXTLEN, iIndex, 0);
            if (lResult != CB_ERR)
            {
                std::string str;
                char* pBuffer = static_cast<char*>(malloc(lResult + 1));
                if (pBuffer != nullptr)
                {
                    std::string str;
                    lResult = ::SendMessageA(hComboBox, CB_GETLBTEXT, iIndex, reinterpret_cast<LPARAM>(pBuffer));
                    if (lResult != CB_ERR)
                    {
                        str = pBuffer;
                    }
                    free(pBuffer);
                    return str;
                }
            }
        }
    }
    return std::string();
}
/*選択項目指定*/
bool combo_box_utility::SetSelectedItem(HWND hComboBox, int iIndex)
{
    if (hComboBox != nullptr)
    {
        LRESULT lResult = ::SendMessage(hComboBox, CB_SETCURSEL, iIndex, 0);
        return iIndex == -1 ? lResult == CB_ERR : lResult == iIndex;
    }
    return false;
}
/*コンボボックス構築*/
bool combo_box_utility::SetupComboBox(HWND hComboBox, const std::unordered_map<int, std::string> &indexedList)
{
    ClearComboBox(hComboBox);

    for (const auto& list : indexedList)
    {
        ::SendMessageA(hComboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(list.second.data()));
    }
    return false;
}

namespace list_view_utility
{
    /*項目数取得*/
    int GetListViewItemCount(HWND hListView)
    {
        LRESULT lResult = ::SendMessageA(hListView, LVM_GETITEMCOUNT, 0, 0);
        return static_cast<int>(lResult);
    }
}
// namespace list_view_utility

/*ListView幅調整*/
void list_view_utility::AdjustListViewWidth(HWND hListView, int iColumnCount, bool bToBeScrolled)
{
    if (hListView != nullptr)
    {
        int iScrollWidth = bToBeScrolled ? static_cast<int>(::GetSystemMetrics(SM_CXVSCROLL) * ::GetDpiForSystem() / 96.f) : 0;

        RECT rect;
        ::GetWindowRect(hListView, &rect);
        int iWindowWidth = rect.right - rect.left;

        LVCOLUMNW lvColumn{};
        lvColumn.mask = LVCF_WIDTH;
        lvColumn.cx = iWindowWidth / iColumnCount - iScrollWidth;
        for (int i = 0; i < iColumnCount; ++i)
        {
            ListView_SetColumn(hListView, i, &lvColumn);
        }
    }
}

bool list_view_utility::AddItemToListView(HWND hListView, const std::vector<std::wstring>& columns, bool bToBottom)
{
    int iItem = GetListViewItemCount(hListView);
    if (iItem == -1)return false;

    LRESULT lResult = -1;
    for (size_t i = 0; i < columns.size(); ++i)
    {
        LVITEM lvItem{};
        lvItem.mask = LVIF_TEXT | LVIF_PARAM;

        lvItem.iItem = bToBottom ? iItem : 0;
        lvItem.iSubItem = static_cast<int>(i);
        lvItem.pszText = const_cast<wchar_t*>(columns.at(i).c_str());

        if (i == 0)
        {
            lResult = ::SendMessage(hListView, LVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&lvItem));
            if (lResult == -1)return false;
            iItem = static_cast<int>(lResult);
        }
        else
        {
            lResult = ::SendMessage(hListView, LVM_SETITEMTEXT, iItem, reinterpret_cast<LPARAM>(&lvItem));
            if (lResult == -1)return false;
        }
    }

    return true;
}
/*リスト項目消去*/
void list_view_utility::ClearListView(HWND hListView)
{
    ListView_DeleteAllItems(hListView);
}

void list_view_utility::CreateSingleList(HWND hListView, const std::vector<std::wstring>& names)
{
    if (hListView != nullptr)
    {
        ClearListView(hListView);
        for (const auto& name : names)
        {
            std::vector<std::wstring> columns;
            columns.push_back(name);
            AddItemToListView(hListView, columns, true);
        }
    }
}

std::wstring list_view_utility::GetListViewItemText(HWND hListView, int iRow, int iColumn)
{
    std::wstring wstrResult;
    if (hListView != nullptr)
    {
        LV_ITEM lvItem{};
        lvItem.iSubItem = iColumn;

        for (int iSize = 256; iSize < 1025; iSize *= 2)
        {
            std::vector<wchar_t> vBuffer;
            vBuffer.resize(iSize);

            lvItem.cchTextMax = iSize;
            lvItem.pszText = vBuffer.data();
            int iLen = static_cast<int>(::SendMessage(hListView, LVM_GETITEMTEXT, iRow, reinterpret_cast<LPARAM>(&lvItem)));
            if (iLen < iSize - 1)
            {
                wstrResult = vBuffer.data();
                break;
            }
        }
    }
    return wstrResult;
}

/*文字列取得*/
std::string edit_box_utility::GetEditBoxText(HWND hEditBoxWnd)
{
    if (hEditBoxWnd != nullptr)
    {
        int iLen = ::GetWindowTextLengthA(hEditBoxWnd); // 終端を含まない
        if (iLen > 0)
        {
            std::string str;
            str.resize(iLen + 1LL);
            LRESULT lResult = ::SendMessageA(hEditBoxWnd, WM_GETTEXT, iLen + 1LL, reinterpret_cast<LPARAM>(str.data()));
            return str;
        }
    }
    return std::string();
}

void edit_box_utility::SetEditBoxText(HWND hEditBoxWnd, const char* pzText)
{
    ::SendMessageA(hEditBoxWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(pzText));
}
/*文字列消去*/
void edit_box_utility::ClearEditBox(HWND hEditBoxWnd)
{
    if (hEditBoxWnd != nullptr)
    {
        ::SendMessageA(hEditBoxWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(""));
    }
}

void list_box_utility::AddListBoxText(HWND hListBoxWnd, const char* pzText)
{
    if (hListBoxWnd != nullptr)
    {
        ::SendMessageA(hListBoxWnd, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pzText));
    }
}

void list_box_utility::ClearListBox(HWND hListBoxWnd)
{
    if (hListBoxWnd != nullptr)
    {
        ::SendMessage(hListBoxWnd, LB_RESETCONTENT, 0, 0);
    }
}

/*EditBox作成*/
void common_control_utility::CreateEditBox(HWND* pEditBoxHandle, const wchar_t* pwzText, HWND hParentWnd, bool bReadOnly)
{
    *pEditBoxHandle = ::CreateWindowEx(0, WC_EDITW, pwzText, (bReadOnly ? ES_READONLY : 0x00) | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);
}
/*StaticText作成*/
void common_control_utility::CreateStaticTextA(HWND* pStaticTextHandle, const char* pzText, HWND hParentWnd)
{
    *pStaticTextHandle = ::CreateWindowExA(0, WC_STATICA, pzText, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);
}
/*Button作成*/
void common_control_utility::CreateButton(HWND* pButtonHandle, const wchar_t* pwzText, HWND hParentWnd, HMENU hControlId)
{
    *pButtonHandle = ::CreateWindowExW(0, WC_BUTTONW, pwzText, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, hParentWnd, hControlId, ::GetModuleHandle(NULL), nullptr);
}
/*ListView作成*/
void common_control_utility::CreateListView(HWND* pListViewHandle, HWND hParentWnd, const std::vector<std::wstring>& columnNames, bool bHasCheckBox)
{
    *pListViewHandle = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_ALIGNLEFT | WS_TABSTOP | LVS_SINGLESEL, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
    if (*pListViewHandle != nullptr)
    {
        ListView_SetExtendedListViewStyle(*pListViewHandle, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | (bHasCheckBox ? LVS_EX_CHECKBOXES : 0));

        LVCOLUMNW lvColumn{};
        lvColumn.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
        lvColumn.fmt = LVCFMT_LEFT;
        for (size_t i = 0; i < columnNames.size(); ++i)
        {
            lvColumn.iSubItem = static_cast<int>(i);
            lvColumn.pszText = const_cast<LPWSTR>(columnNames.at(i).data());
            ListView_InsertColumn(*pListViewHandle, i, &lvColumn);
        }
    }
}
/*ListBox作成*/
void common_control_utility::CreateListBox(HWND* pListBoxHandle, HWND hParentWnd)
{
    *pListBoxHandle = ::CreateWindowExW(0, WC_LISTBOX, L"ListBox", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_SORT | LBS_NOINTEGRALHEIGHT | WS_VSCROLL, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
}