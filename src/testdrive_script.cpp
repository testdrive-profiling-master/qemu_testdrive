//================================================================================
// Copyright (c) 2013 ~ 2026. HyungKi Jeong(clonextop@gmail.com)
// Freely available under the terms of the 3-Clause BSD License
// (https://opensource.org/licenses/BSD-3-Clause)
//
// Redistribution and use in source and binary forms,
// with or without modification, are permitted provided
// that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.
//
// Title : QEMU for TestDrive
// Rev.  : 4/23/2026 Thu (clonextop@gmail.com)
//================================================================================
#include "testdrive_device.h"

static TestDrive *__pTestDrive = NULL;
int				  luaopen_lfs(lua_State *L);

class lua_cstring : public cstring
{
	int iTokenizePos;

public:
	lua_cstring(void)
	{
		iTokenizePos = 0;
	}
	lua_cstring(LuaRef s)
	{
		if (!s.isNil()) {
			m_sStr = s.tostring();
		}

		iTokenizePos = 0;
	}
	lua_cstring(const cstring &s)
	{
		m_sStr		 = s.c_str();
		iTokenizePos = 0;
	}
	lua_cstring(const lua_cstring &s)
	{
		m_sStr		 = s.c_str();
		iTokenizePos = 0;
	}
	~lua_cstring(void) {}

	int Compare(const char *s)
	{
		return cstring::Compare(s);
	}
	bool CompareFront(const char *s) const
	{
		return cstring::CompareFront(s);
	}
	bool CompareBack(const char *s) const
	{
		return cstring::CompareBack(s);
	}
	int RetrieveTag(LuaRef t) const
	{
		if (t.isTable()) {
			for (int i = 1;; i++) {
				LuaRef v = t[i];
				if (!v.isString())
					break;
				if (m_sStr == (string)v)
					return i;
			}
		}
		return 0;
	}
	bool IsEmpty(void) const
	{
		return cstring::IsEmpty();
	}
	bool CutFront(const char *s, bool bRecursive = false)
	{
		iTokenizePos = 0;
		return cstring::CutFront(s, bRecursive);
	}
	bool CutBack(const char *s, bool bRecursive = false)
	{
		iTokenizePos = 0;
		return cstring::CutBack(s, bRecursive);
	}
	bool DeleteFront(const char *s)
	{
		iTokenizePos = 0;
		return cstring::DeleteFront(s);
	}
	bool DeleteBack(const char *s)
	{
		iTokenizePos = 0;
		return cstring::DeleteBack(s);
	}
	bool DeleteBlock(const char *sExpression, int iPos = 0)
	{
		iTokenizePos = 0;
		return cstring::DeleteBlock(sExpression, iPos);
	}
	void MakeUpper(void)
	{
		cstring::MakeUpper();
	}
	void MakeLower(void)
	{
		cstring::MakeLower();
	}
	void MakeFullPath(void)
	{
		cstring::MakeFullPath();
	}
	bool Replace(const char *sSearch, const char *sReplace, bool bRecursive = false)
	{
		iTokenizePos = 0;
		return cstring::Replace(sSearch, sReplace, bRecursive);
	}
	bool ReplaceVariable(const char *sSearch, const char *sReplace)
	{
		iTokenizePos = 0;
		return cstring::ReplaceVariable(sSearch, sReplace);
	}
	void TrimLeft(const char *sDelim)
	{
		iTokenizePos = 0;
		cstring::TrimLeft(sDelim);
	}
	void TrimRight(const char *sDelim)
	{
		iTokenizePos = 0;
		cstring::TrimRight(sDelim);
	}
	void Trim(const char *sDelim)
	{
		iTokenizePos = 0;
		cstring::Trim(sDelim);
	}
	bool ReadFile(const char *sFile, bool bUseComment = false)
	{
		TextFile f;
		cstring	 sFileName(sFile);
		clear();
		sFileName.ChangeCharsetToANSI();

		if (f.Open(sFileName.c_str())) {
			f.GetAll(*(cstring *)this, bUseComment);
			return true;
		}

		return false;
	}
	int GetTokenizeLuaPos(void) const
	{
		return iTokenizePos;
	}
	void SetTokenizeLuaPos(int iPos)
	{
		iTokenizePos = iPos;
	}
	lua_cstring TokenizeLua(const char *sDelim)
	{
		return cstring::Tokenize(iTokenizePos, sDelim);
	}

	cstring		m_sVariable;
	lua_cstring TokenizeVariable(const char *sExpression)
	{
		cstring sTok;

		if (iTokenizePos >= 0) {
			int iPrevPos = iTokenizePos;
			iTokenizePos = FindVariableString(m_sVariable, sExpression, iTokenizePos);
			sTok		 = m_sStr.c_str() + iPrevPos;

			if (iTokenizePos >= 0) {
				sTok = m_sStr.c_str() + iPrevPos;
				sTok.erase(iTokenizePos - iPrevPos, -1);
			} else {
				m_sVariable.clear();
			}
		} else {
			m_sVariable.clear();
			sTok.clear();
		}

		return sTok;
	}

	lua_cstring GetVariable(void)
	{
		return m_sVariable;
	}

	int Length(void) const
	{
		return cstring::Length();
	}
	void Set(const char *sStr)
	{
		iTokenizePos = 0;
		cstring::Set(sStr);
	}

	int CheckFileExtensionLua(const char *sExtList)
	{
		if (sExtList) {
			const char	*sDelim = " .,;";
			cstring		 sExts(sExtList);
			list<string> sList;
			{
				// make ext list
				int iPos = 0;

				while (1) {
					cstring sExt = sExts.Tokenize(iPos, sDelim);

					if (iPos > 0) {
						sList.push_back(sExt.c_str());
					} else
						break;
				}
			}

			if (sList.size()) {
				const char **sExtPrivateList = new const char *[sList.size() + 1];
				int			 t				 = 0;

				for (auto &i : sList) {
					sExtPrivateList[t] = i.c_str();
					t++;
				}

				sExtPrivateList[t] = NULL;
				int id			   = CheckFileExtension(sExtPrivateList);
				delete[] sExtPrivateList;
				return id;
			}
		}

		return -1;
	}

	bool GetEnvironment(const char *sKey)
	{
		return cstring::GetEnvironment(sKey);
	}

	void SetEnvironment(const char *sKey)
	{
		cstring::SetEnvironment(sKey);
	}

	void FormatDate(const char *sFormat, int iDayShift)
	{
		char	  sTime[1024];
		time_t	  now = time(0);
		struct tm t	  = *localtime(&now);

		if (iDayShift) {
			t.tm_mday += iDayShift;
			mktime(&t);
		}

		strftime(sTime, sizeof(sTime), sFormat, &t);
		m_sStr = sTime;
	}

	bool ChangeCharset(const char *szSrcCharset, const char *szDstCharset)
	{
		return cstring::ChangeCharset(szSrcCharset, szDstCharset);
	}
	bool ChangeCharsetToUTF8(void)
	{
		return cstring::ChangeCharsetToUTF8();
	}
	bool ChangeCharsetToANSI(void)
	{
		return cstring::ChangeCharsetToANSI();
	}

	void Append(const char *sStr)
	{
		cstring::Append(sStr);
	}
	inline const char *c_str(void) const
	{
		return m_sStr.c_str();
	}
	inline char get(int iPos) const
	{
		if (iPos < 0 || iPos >= (int)m_sStr.length())
			return 0;

		return m_sStr.c_str()[iPos];
	}
	inline int find_ch(char ch, int pos = 0)
	{
		return m_sStr.find(ch, pos);
	}
	inline int find(const char *s, LuaRef pos)
	{
		int ipos = 0;
		if (pos.isNumber()) {
			ipos = (int)pos;
		}
		return m_sStr.find(s, ipos);
	}
	inline int rfind(const char *s)
	{
		return m_sStr.rfind(s);
	}
	inline int rfind_ch(char ch)
	{
		return m_sStr.rfind(ch);
	}
	inline int size(void)
	{
		return m_sStr.size();
	}
	inline int length(void)
	{
		return m_sStr.length();
	}
	inline void clear(void)
	{
		m_sStr.clear();
		iTokenizePos = 0;
	}
	inline void erase(int iPos, int iSize)
	{
		m_sStr.erase(iPos, iSize);
	}
	inline void insert(int iPos, const char *s)
	{
		if (iPos < iTokenizePos)
			iTokenizePos += strlen(s);

		m_sStr.insert(iPos, s);
	}
	inline void replace(int iPos, int iSize, const char *s)
	{
		m_sStr.replace(iPos, iSize, s);
	}
};

static void __LOGI(const char *sLog)
{
	cstring s(sLog);
	s.ChangeCharsetToANSI();
	LOGI(s.c_str());
}

static void __LOGE(const char *sLog)
{
	cstring s(sLog);
	s.ChangeCharsetToANSI();
	LOGE(s.c_str());
}

static void __LOGW(const char *sLog)
{
	cstring s(sLog);
	s.ChangeCharsetToANSI();
	LOGW(s.c_str());
}

bool TestDrive::CreateBAR(const char *space_type, uint64_t byte_size, bool b64bit, bool bPrefetchable, uint64_t bind_address)
{
	const char		 *__address_space_type[] = {"io", "memory"};
	TESTDRIVE_PCI_BAR bar					 = {0};

	if (space_type) {
		cstring sType(space_type);
		int		iType = sType.RetrieveTag(__address_space_type, ARRAY_SIZE(__address_space_type));
		if (iType < 0) {
			LOGE("BAR type must be 'io' or 'memory'.");
			return false;
		}
		bar.option.bar_type = iType;
	}
	for (auto &i : m_BARs) {
		bar.option.bar_id += i.second.option.b64bit ? 2 : 1;
	}
	bar.pTestDrive			 = this;
	bar.byte_size			 = byte_size;
	bar.option.b64bit		 = b64bit;
	bar.option.bPrefetchable = bPrefetchable;
	bar.bind_address		 = bind_address;

	if (bar.option.bar_id + (bar.option.b64bit ? 2 : 1) > 6) {
		LOGE("No more available BAR left.");
		return false;
	}

	{
		cstring sBarName;
		sBarName.Format("testdrive_bar%d", bar.option.bar_id);
		strcpy(bar.name, sBarName.c_str());
	}

	m_BARs[bar.option.bar_id]				   = bar;
	testdrive_param.pci.bar[bar.option.bar_id] = &m_BARs[bar.option.bar_id];

	LOGI(
		"TestDrive.BAR[%d] : Type(%s) byte_size(0x%llX) b64bit(%d) bPrefetchable(%d) bind_address(0x%llX)", bar.option.bar_id,
		__address_space_type[bar.option.bar_type], byte_size, b64bit, bPrefetchable, bind_address);

	if (((byte_size - 1) & bind_address) != 0) {
		LOGW(
			"BAR binding address(0x%llX) must be aligned by byte_size(0x%llX).\n    It can only be available on a virtual machine environment.",
			bind_address, byte_size);
	}
	return true;
}

bool TestDrive::EnableMSI(int iVectorCount, bool bMaskPerVector)
{
	if (iVectorCount < 0 || iVectorCount >= 32) {
		LOGE("MSI count(%d) is out of range (0~32).", iVectorCount);
		return false;
	}

	testdrive_param.pci.msi.vector_count	= iVectorCount;
	testdrive_param.pci.msi.per_vector_mask = bMaskPerVector;
	return true;
}

bool TestDrive::EnableDisplay(int width, int height, LuaRef disp_address)
{
	if (width && height) {
		if (!m_pSystem) {
			LOGW("Can't enable display, you must load your system module first.!");
			return false;
		}
		display.width		= width;
		display.height		= height;
		display.byte_stride = display.width * 4;
		uint64_t addr		= disp_address.isNil() ? m_pSystem->GetMemoryBase() : (uint64_t)disp_address;
		display.pBuffer		= m_pSystem->GetMemoryPointer(addr);
	} else {
		memset(&display, 0, sizeof(TESTDRIVE_DISPLAY));
	}
	return true;
}

bool TestDrive::RunScript(const char *sScriptFileName)
{
	if (!m_pLua) {
		if ((m_pLua = luaL_newstate()) != NULL) {
			luaL_openlibs(m_pLua);
			luaopen_lfs(m_pLua);

			// register function/class
			getGlobalNamespace(m_pLua)
				// name space functions
				.beginClass<TextFile>("TextFile")
				.addConstructor<void (*)(void)>()
				.addFunction("Open", &TextFile::Open)
				.addFunction("Create", &TextFile::Create)
				.addFunction("Close", &TextFile::Close)
				.addFunction("Put", &TextFile::Puts)
				.addFunction("Get", &TextFile::Gets)
				.addFunction(
					"GetAll", std::function<string(TextFile * pFile, LuaRef bUseComment)>([](TextFile *pFile, LuaRef bUseComment) -> string {
						cstring sContents;
						pFile->GetAll(sContents, bUseComment.isBool() ? (bool)bUseComment : false);
						return sContents.c_string();
					}))
				.addFunction("LineNumber", &TextFile::LineNumber)
				.addFunction("IsOpen", &TextFile::IsOpen)
				.addFunction("IsEOF", &TextFile::IsEOF)
				.endClass()
				.beginClass<lua_cstring>("String")
				.addConstructor<void (*)(LuaRef s)>()
				.addFunction("Replace", &lua_cstring::Replace)
				.addFunction("ReplaceVariable", &lua_cstring::ReplaceVariable)
				.addProperty("s", &lua_cstring::c_str, &lua_cstring::Set)
				.addFunction("Append", &lua_cstring::Append)
				.addFunction("Length", &lua_cstring::Length)
				.addFunction("Compare", &lua_cstring::Compare)
				.addFunction("CompareFront", &lua_cstring::CompareFront)
				.addFunction("CompareBack", &lua_cstring::CompareBack)
				.addFunction("RetrieveTag", &lua_cstring::RetrieveTag)
				.addFunction("IsEmpty", &lua_cstring::IsEmpty)
				.addFunction("CutFront", &lua_cstring::CutFront)
				.addFunction("CutBack", &lua_cstring::CutBack)
				.addFunction("ReadFile", &lua_cstring::ReadFile)
				.addFunction("Trim", &lua_cstring::Trim)
				.addFunction("TrimLeft", &lua_cstring::TrimLeft)
				.addFunction("TrimRight", &lua_cstring::TrimRight)
				.addFunction("Tokenize", &lua_cstring::TokenizeLua)
				.addFunction("TokenizeVariable", &lua_cstring::TokenizeVariable)
				.addFunction("GetVariable", &lua_cstring::GetVariable)
				.addProperty("TokenizePos", &lua_cstring::GetTokenizeLuaPos, &lua_cstring::SetTokenizeLuaPos)
				.addFunction("DeleteFront", &lua_cstring::DeleteFront)
				.addFunction("DeleteBack", &lua_cstring::DeleteBack)
				.addFunction("DeleteBlock", &lua_cstring::DeleteBlock)
				.addFunction("MakeUpper", &lua_cstring::MakeUpper)
				.addFunction("MakeLower", &lua_cstring::MakeLower)
				.addFunction("MakeFullPath", &lua_cstring::MakeFullPath)
				.addFunction("CheckFileExtension", &lua_cstring::CheckFileExtensionLua)
				.addFunction("GetEnvironment", &lua_cstring::GetEnvironment)
				.addFunction("SetEnvironment", &lua_cstring::SetEnvironment)
				.addFunction("FormatDate", &lua_cstring::FormatDate)
				.addFunction("ChangeCharset", &lua_cstring::ChangeCharset)
				.addFunction("ChangeCharsetToUTF8", &lua_cstring::ChangeCharsetToUTF8)
				.addFunction("ChangeCharsetToANSI", &lua_cstring::ChangeCharsetToANSI)
				.addFunction("insert", &lua_cstring::insert)
				.addFunction("clear", &lua_cstring::clear)
				.addFunction("find", &lua_cstring::find)
				.addFunction("rfind", &lua_cstring::rfind)
				.addFunction("at", &lua_cstring::get)
				.addFunction("erase", &lua_cstring::erase)
				.addFunction("insert", &lua_cstring::insert)
				.endClass()
				.beginClass<TestDrive>("TestDrive")
				.addFunction("CreateBAR", &TestDrive::CreateBAR)
				.addFunction("EnableMSI", &TestDrive::EnableMSI)
				.addFunction("EnableDisplay", &TestDrive::EnableDisplay)
				.addFunction("LoadSystemModule", &TestDrive::LoadSystemModule)
				.endClass()
				.addFunction("LOGI", __LOGI)
				.addFunction("LOGE", __LOGE)
				.addFunction("LOGW", __LOGW)
				.addFunction("TestDriveDevice", std::function<TestDrive *(void)>([](void) -> TestDrive * {
								 return __pTestDrive;
							 }))
				.addFunction("GetConfiguration", &GetConfiguration);
		} else {
			LOGE("Can't create Lua engine.");
			return false;
		}
	}

	__pTestDrive = this;

	// run script
	if (sScriptFileName && luaL_dofile(m_pLua, sScriptFileName)) {
		// got error on script
		const char *sError = luaL_checkstring(m_pLua, -1);
		LOGE("Error on running script : %s", sError);
		lua_pop(m_pLua, 1); // pop out error message
							// return false;
		return false;
	}
	return true;
}
