#pragma once

#include <memory>
#include <vector>
#include <string>
#include "BaseEntity.h"
#include <functional>
#include "PenDataHandleGlobal.h"

class DLL_EXPORT PenDataHandle
{
public:
	virtual void handleDot(double X, double Y, int bookId, int pageId, int type, int force, long long timeStamp) = 0;
	virtual void addOnPenDown(std::function<void(const std::string&)>) = 0;
	virtual void addOnClear(std::function<void(const std::string&)>) = 0;
	virtual void addOnResult(std::function<void(const std::string&)>) = 0;
	virtual void addOnNewPoint(std::function<void(const std::string& ,int, std::vector<std::shared_ptr<Point>>&)> d) = 0;
	virtual void addOnNewGrid(std::function<void(const std::string& ,int, int, int, int, int)>) = 0;
	virtual void addOnStrokeTimeInterval(std::function<void(const std::string&, int, long long)>) = 0;
	virtual void addOnWordTimeInterval(std::function<void(const std::string&, long long)>) = 0;

private:

};