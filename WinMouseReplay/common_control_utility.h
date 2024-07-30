#ifndef COMMON_CONTROL_UTILITY_H_
#define COMMON_CONTROL_UTILITY_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <unordered_map>

namespace combo_box_utility
{
	std::string FormatInt(int iValue, int iLength);

	std::string GetComboBoxText(HWND hComboBox);
	bool SetSelectedItem(HWND hComboBox, int iIndex);
	bool SetupComboBox(HWND hComboBox, const std::unordered_map<int, std::string> &indexedList);
}

namespace list_view_utility
{
	void AdjustListViewWidth(HWND hListView, int iColumnCount, bool bToBeScrolled = false);
	bool AddItemToListView(HWND hListView, const std::vector<std::wstring>& columns, bool bToBottom = true);

	void ClearListView(HWND hListView);
	void CreateSingleList(HWND hListView, const std::vector<std::wstring>& names);

	std::wstring GetListViewItemText(HWND hListView, int iRow, int iColumn);
}

namespace edit_box_utility
{
	std::string GetEditBoxText(HWND hEditBoxWnd);
	void SetEditBoxText(HWND hEditBoxWnd, const char* pzText);
	void ClearEditBox(HWND hEditBoxWnd);
}

namespace list_box_utility
{
	void AddListBoxText(HWND hListBoxWnd, const char* pzText);
	void ClearListBox(HWND hListBoxWnd);
}

namespace common_control_utility
{
	void CreateEditBox(HWND* pEditBoxHandle, const wchar_t* pwzText, HWND hParentWnd, bool bReadOnly = false);
	void CreateStaticTextA(HWND* pStaticTextHandle, const char* pzText, HWND hParentWnd);
	void CreateButton(HWND* pButtonHandle, const wchar_t* pwzText, HWND hParentWnd, HMENU hControlId);

	void CreateListView(HWND* pListViewHandle, HWND hParentWnd, const std::vector<std::wstring>& columnNames, bool bHasCheckBox = false);
	void CreateListBox(HWND* pListBoxHandle, HWND hParentWnd);
}

#endif // COMMON_CONTROL_UTILITY_H_
