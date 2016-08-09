#include "stdafx.h"
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <stdlib.h>
#include <vcclr.h>
#include <strsafe.h>
#include <iostream>
#pragma comment(lib, "User32.lib")

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace std;

ref class FileArray
{
private:
	ArrayList^ StringList = gcnew ArrayList();
public:
	void PrintToOutput()
	{
		// Time to write down the stuff!
		String^ fileName = gcnew System::String("output.res");
		StreamWriter^ WriteFile = gcnew StreamWriter(fileName);
		WriteFile->WriteLine("\"resources\"");
		WriteFile->WriteLine("{");
		for (int i = 0; i < StringList->Count; i++)
			WriteFile->WriteLine("	\"{0}\" \"file\"", StringList[i]);
		WriteFile->WriteLine("}");
		WriteFile->Close();
	}

	void WriteArray(String^ File)
	{
		StringList->Add(File);
	}

	void ArrayDestroy()
	{
		for (int i = 0; i < StringList->Count; i++)
			StringList->Remove(StringList[i]);
	}

	void ReadDirectory(const wchar_t *Dirname, const wchar_t *ResourceRootDir)
	{
		WIN32_FIND_DATA ffd;
		TCHAR szDir[MAX_PATH];
		HANDLE hFind = INVALID_HANDLE_VALUE;
		DWORD dwError = 0;

		StringCchCopy(szDir, MAX_PATH, Dirname);
		StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

		// Find the first file in the directory.
		hFind = FindFirstFile(szDir, &ffd);

		// Nothing found, dead end.
		if (INVALID_HANDLE_VALUE == hFind)
		{
			DisplayErrorBox(TEXT("FindFirstFile"));
			return;
		}

		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// We don't want to go back, or check the current files again.
				String^ DirName = gcnew System::String(ffd.cFileName);
				String^ ResourceDirName = gcnew System::String(ResourceRootDir);
				String^ RootDir = gcnew System::String(Dirname);
				if (DirName->Equals(".") || DirName->Equals(".."))
					continue;
				RootDir += DirName + "/";

				// We don't want to include the same dir twice
				if (ResourceDirName != DirName)
					ResourceDirName += DirName + "/";

				pin_ptr<const wchar_t> SetDir = PtrToStringChars(RootDir);
				pin_ptr<const wchar_t> SetResourceRootDir = PtrToStringChars(ResourceDirName);
				ReadDirectory(SetDir, SetResourceRootDir);
			}
			else
			{
				String^ Filename = gcnew System::String(ffd.cFileName);
				String^ RootDir = gcnew System::String(ResourceRootDir);
				RootDir += Filename;

				pin_ptr<const wchar_t> PrintFile = PtrToStringChars(RootDir);
				printf_s("Added \"%S\" to resource file \"output.res\"\n", PrintFile);
				WriteArray(RootDir);
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		// No more files :(
		dwError = GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
		{
			DisplayErrorBox(TEXT("FindFirstFile"));
		}

		FindClose(hFind);
	}

	void DisplayErrorBox(LPTSTR lpszFunction)
	{
		// Retrieve the system error message for the last-error code

		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		// Display the error message and clean up

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
			(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,
			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("%s failed with error %d: %s"),
			lpszFunction, dw, lpMsgBuf);
		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
	}
};

int main(int argc, char *argv[], char *envp[])
{
	bool FoundPath = false;

	// path found
	if (argc >= 2)
		FoundPath = true;

	// Read the folder
	if (FoundPath)
	{
		// Lets get rid of backslashes
		String^ PathArgument = gcnew System::String(argv[1]);

		// arg isn't more than 3 letters?
		if (PathArgument->Length < 3)
			PathArgument = "./content/";

		String^ correctString = PathArgument->Replace("\\", "/")->Replace("\"", "");

		// Check if the last letter is forwardslash
		// If not, lets add it! This will make sure the path isn't incorrect
		String^ GrabLastString = correctString->Substring(correctString->Length - 1, 1);

		if (GrabLastString != "/")
		{
			correctString->Substring(0, correctString->Length - 1);
			correctString += "/";
		}

		// Formating stage
		pin_ptr<const wchar_t> SetDir = PtrToStringChars(correctString);
		pin_ptr<const wchar_t> SetDir_temp = PtrToStringChars("");
		FileArray^ farray = gcnew FileArray();
		farray->ReadDirectory(SetDir, SetDir_temp);
		farray->PrintToOutput();
		farray->ArrayDestroy();
	}
	else
	{
		cout << "\nERROR"
			<< "\nPlease specify a directory!\n"
			<< "\"hl2resgen.exe <folder>\"\n\n\n";
		return 0;
	}
}
