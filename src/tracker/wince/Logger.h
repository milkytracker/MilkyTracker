// About this class
// ----------------------------------------------------------------------
// This class is a very simple logger class. It simply logs strings
// in a vector, provides access to the log entries  and optionally 
// exports the log into a text file.
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <windows.h>

class SimpleString;
template <class T> class SimpleVector; 

class CLogger
{
private:
	// the vector which contains the log data
	SimpleVector<SimpleString>* m_log;

	LPCTSTR fileName;

public:
	// construct logger instance
	CLogger();
	// construct logger instance which writes out to filename on destruction
	CLogger(LPCTSTR fileName);
	// destruct logger instance
	~CLogger();

	// clear log
	void Clear();
	// log a string
	void Log(const SimpleString& logData);

	// return size of log
	unsigned int GetSize() const;
	// get log entry
	const SimpleString& GetEntry(unsigned int i) const;

	// is this log empty?
	bool IsEmpty() const { return m_log ? GetSize() == 0 : true; }

	// export log file into a text file
	bool Export(LPCTSTR fileName) const;
};

#endif
