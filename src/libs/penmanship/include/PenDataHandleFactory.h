#pragma once
#include "PenDataHandle.h"
#include "PenDataHandleGlobal.h"
#include <string>

class DLL_EXPORT PenDataHandleFactory
{
public:
	PenDataHandleFactory();
	~PenDataHandleFactory();

	static PenDataHandle* createHandle(const std::string& mark);

private:

};