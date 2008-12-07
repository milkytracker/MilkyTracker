#include "Logger.h"
#include "Simple.h"

// construct logger
CLogger::CLogger() :
	fileName(NULL)
{
	m_log = new SimpleVector<SimpleString>();
}

CLogger::CLogger(LPCTSTR fileName) :
	fileName(fileName)	
{
	m_log = new SimpleVector<SimpleString>();
}

// destroy logger
CLogger::~CLogger()
{
	if (fileName)
		Export(fileName);
	delete m_log;
}

// clear logger
void CLogger::Clear()
{
	m_log->clear();
}

// log string
void CLogger::Log(const SimpleString& logData)
{
	m_log->add(new SimpleString(logData));
}

// how many strings does the logger contain?
unsigned int CLogger::GetSize() const
{
	return m_log->size();
}

// get single entry from logger
const SimpleString& CLogger::GetEntry(unsigned int i) const
{
	return *m_log->get(i);
}

// export this log to a text file
bool CLogger::Export(LPCTSTR fileName) const
{
	// create file
	HANDLE handle = ::CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	// file couldn't be created
	if (INVALID_HANDLE_VALUE == handle)
		return false;

	// write out all strings in ANSI format
	for (int i = 0; i < m_log->size(); i++)
	{
		const SimpleString& strData = *m_log->get(i);

		// How many memory do we need for the ANSI string?
		int len = strData.length()*2 + 4;
		
		// allocate memory
		char* szOut = new char[len];

		// convert Unicode string to ANSI using default encoding
		int	res = ::WideCharToMultiByte(CP_ACP, 0, strData, strData.length(), szOut, len, NULL, NULL);

		// if successfully converted append NEWLINE & CR
		if (res)
		{
			szOut[res] = '\r';
			szOut[res+1] = '\n';
			// terminate string
			szOut[res+2] = '\0';
		
			// write out string
			unsigned long NumberOfBytesWritten;
			BOOL bResult = ::WriteFile(handle, szOut, res+2, &NumberOfBytesWritten, NULL);
			// flush buffers just in case
			::FlushFileBuffers(handle);
		}

		// ANSI string no longer needed
		delete[] szOut;
	}

	// close file
	::CloseHandle(handle);

	return true;
}
