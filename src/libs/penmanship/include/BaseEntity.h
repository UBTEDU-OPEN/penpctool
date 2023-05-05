#pragma once
#include <memory>
#include <vector>
#include "PenDataHandleGlobal.h"

class DLL_EXPORT Point
{
public:
	Point();

	Point(double x,double y, long long time,short type,double width);

	~Point();

	void setX(double x);

	double getX(void);

	void setY(double y);

	double getY(void);

	void setTime(long long time);

	long long getTime(void);

	void setType(int type);

	int getType();

	void setWidth(double width);

	double getWidth();

private:
	double			x;
	double			y;
	long long		time;
	int				type;
	double          width;
};

class DLL_EXPORT Stroke
{
public:
	Stroke();
	~Stroke();

	void addPoint(std::shared_ptr<Point>& point);

	void setPointList(std::vector<std::shared_ptr<Point>>& pList,bool isClear);

	std::vector<std::shared_ptr<Point>>& getPointList();

	void setStrokeId(int strokeId);

	int getStrokeId(void);

private:
	int				strokeId;
	std::vector<std::shared_ptr<Point>>  pointList;
};

