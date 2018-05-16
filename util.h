#pragma once

// version stuff from skse64_loader_common
static bool GetFileVersion(const char * path, VS_FIXEDFILEINFO * info, std::string * outProductName, std::string * outProductVersion)
{
	bool result = false;

	UInt32	versionSize = GetFileVersionInfoSize(path, NULL);
	if (!versionSize)
	{
		_ERROR("GetFileVersionInfoSize failed (%08X)", GetLastError());
		return false;
	}

	UInt8	* versionBuf = new UInt8[versionSize];
	if (versionBuf)
	{
		if (GetFileVersionInfo(path, NULL, versionSize, versionBuf))
		{
			VS_FIXEDFILEINFO	* retrievedInfo = NULL;
			UInt32				realVersionSize = sizeof(VS_FIXEDFILEINFO);

			if (VerQueryValue(versionBuf, "\\", (void **)&retrievedInfo, (PUINT)&realVersionSize) && retrievedInfo)
			{
				*info = *retrievedInfo;
				result = true;
			}
			else
			{
				_ERROR("VerQueryValue failed (%08X)", GetLastError());
			}

			if (outProductName)
			{
				// try to get the product name, failure is ok
				char * productName = NULL;
				UInt32 productNameLen = 0;
				if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductName", (void **)&productName, (PUINT)&productNameLen) && productNameLen && productName)
				{
					*outProductName = productName;
				}
			}

			{
				char * productVersion = NULL;
				UInt32 productVersionLen = 0;
				if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductVersion", (void **)&productVersion, (PUINT)&productVersionLen) && productVersionLen && productVersion)
				{
					*outProductVersion = productVersion;
				}
			}
		}
		else
		{
			_ERROR("GetFileVersionInfo failed (%08X)", GetLastError());
		}

		delete[] versionBuf;
	}

	return result;
}

static bool VersionStrToInt(const std::string & verStr, UInt64 * out)
{
	UInt64 result = 0;
	int parts[4];

	if (sscanf_s(verStr.c_str(), "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]) != 4)
		return false;

	for (int i = 0; i < 4; i++)
	{
		if (parts[i] > 0xFFFF)
			return false;

		result <<= 16;
		result |= parts[i];
	}

	*out = result;

	return true;
}