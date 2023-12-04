
#include "file_operation.h"

/*展開ファイル選択ダイアログ*/
wchar_t* SelectTextFileToOpen()
{
	ComInit init;
	CComPtr<IFileOpenDialog> pFileDialog;
	HRESULT hr = pFileDialog.CoCreateInstance(CLSID_FileOpenDialog);

	if (SUCCEEDED(hr)) {
		COMDLG_FILTERSPEC filter[1]{};
		filter[0].pszName = L"文書形式";
		filter[0].pszSpec = L"*.txt;*.bin;*.dat;";
		hr = pFileDialog->SetFileTypes(1, filter);
		if (SUCCEEDED(hr))
		{
			FILEOPENDIALOGOPTIONS opt{};
			pFileDialog->GetOptions(&opt);
			pFileDialog->SetOptions(opt | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

			if (SUCCEEDED(pFileDialog->Show(nullptr)))
			{
				CComPtr<IShellItem> pSelectedItem;
				pFileDialog->GetResult(&pSelectedItem);

				wchar_t* pPath;
				pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

				return pPath;
			}
		}
	}

	return nullptr;
}
/*保存先選択ダイアログ*/
wchar_t* SelectTextFileToSave()
{
	ComInit init;
	CComPtr<IFileSaveDialog> pFileDialog;
	HRESULT hr = pFileDialog.CoCreateInstance(CLSID_FileSaveDialog);

	if (SUCCEEDED(hr)) 
	{
		COMDLG_FILTERSPEC filter[1]{};
		filter[0].pszName = L"文書形式";
		filter[0].pszSpec = L"*.txt;*.bin;*.dat;";
		hr = pFileDialog->SetFileTypes(1, filter);
		if (SUCCEEDED(hr))
		{
			pFileDialog->SetFileName(L"1.txt");

			FILEOPENDIALOGOPTIONS opt{};
			pFileDialog->GetOptions(&opt);
			pFileDialog->SetOptions(opt | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

			if (SUCCEEDED(pFileDialog->Show(nullptr)))
			{
				CComPtr<IShellItem> pSelectedItem;
				pFileDialog->GetResult(&pSelectedItem);

				wchar_t* pPath;
				pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

				return pPath;
			}
		}
	}

	return nullptr;
}
