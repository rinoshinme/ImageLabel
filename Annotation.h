#pragma once

#include <opencv2\core\core.hpp>
#include <string>
#include <vector>

#include "Annotation2.h"

struct FaultInfo
{
	std::string name;
	cv::Rect bbox;
	FaultInfo(const std::string& n, const cv::Rect& r)
		:name(n), bbox(r) {}
	FaultInfo(){}
};

class CAnnotation
{
public:
	CAnnotation()
	{
		name_ = "";
		bbox_ = cv::Rect(0, 0, 0, 0);
	}

	CAnnotation(const std::string& name, const cv::Rect& box)
	{
		name_ = name;
		bbox_ = box;
	}

	std::string GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

	cv::Rect GetBaseBox() const { return bbox_; }
	void SetBaseBox(const cv::Rect& rect) { bbox_ = rect; }

	void AddFault(const std::string& name, const cv::Rect& rect)
	{
		FaultInfo info;
		info.name = name;
		info.bbox = rect;
		faults_.push_back(info);
	}

	int GetFaultNum() const { return int(faults_.size()); }
	cv::Rect GetFaultBox(int index) const { return faults_[index].bbox; }
	std::string GetFaultName(int index) const { return faults_[index].name; }
	void SetFaultBox(int index, const cv::Rect& rect) { faults_[index].bbox = rect; }
	void SetFaultName(int index, const std::string& name) { faults_[index].name = name; }

	void DeleteFault(int index)
	{
		if (index < 0 || index >= int(faults_.size()))
			return;
		std::vector<FaultInfo>::iterator it = faults_.begin();
		for (int i = 0; i < index; ++i)
			++it;
		faults_.erase(it);
	}

	void SetValidState(bool valid) { valid_ = valid; }
	bool GetValidState() const { return valid_; }

private:
	bool valid_;
	std::string name_;
	cv::Rect bbox_;
	std::vector<FaultInfo> faults_;
};

void SaveToXML(const std::vector<CAnnotation>& annotations, const std::string& srcfile, 
	const std::string& file, int width = 0, int height = 0, int depth = 0);
