#include <Windows.h>
#include "misc.h"
#include "FileTransfer.h"

bool FileTransfer::open_file(HWND hwnd)
{
	// Select file
	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"All Files\0*.*\0\0";
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.lpstrFileTitle = fileinfo.filename;
	ofn.nMaxFileTitle = sizeof(fileinfo.filename);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	if (!GetOpenFileNameW(&ofn))
		return false;

	// Open file and get size
	file = CreateFileW(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LARGE_INTEGER filesize;

	GetFileSizeEx(file, &filesize);
	fileinfo.filesize = (DWORD)filesize.QuadPart;
	return true;
}

bool FileTransfer::save_file(HWND hwnd)
{
	// Initialize savepath
	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	if (!GetSaveFileNameW(&ofn))
		return false;	

	file = CreateFileW(filepath, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	return true;
}

void FileTransfer::load_fileinfo(FileInfo* fileinfo)
{
	memcpy(filepath, fileinfo->filename, MAX_PATH / 2);
	memcpy(this->fileinfo.filename, fileinfo->filename, MAX_PATH / 2);
	this->fileinfo.filesize = fileinfo->filesize;
}

DWORD FileTransfer::read_file(BYTE* data, int len)
{
	DWORD byteread = 0;
	if (ReadFile(file, data, len, &byteread, NULL))
		return byteread;
	return 0;
}

bool FileTransfer::write_file(BYTE* data, int len)
{
	return WriteFile(file, data, len, NULL, NULL);
}

FileTransfer::~FileTransfer()
{
	CloseHandle(file);
}
