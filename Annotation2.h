#ifndef ANNOTATION2_H
#define ANNOTATION2_H

#include <opencv2/core/core.hpp>
#include <string>
#include <vector>

#include "tinyxml/tinyxml.h"

struct Fault
{
public:
	std::string name;
	cv::Rect bndbox;
	bool valid;

	Fault() : name(""), bndbox(cv::Rect(0, 0, 0, 0)), valid(false){}
	Fault(const std::string& n, const cv::Rect& r)
		:name(n), bndbox(r), valid(true) {}
};

class AnnotObject
{
public:
	std::string name;
	std::string pose;
	bool truncated;
	int difficult;
	cv::Rect bndbox;
	std::vector<Fault> faults;
	bool valid;
	AnnotObject()
	{
		name = "";
		pose = "Unspecified";
		truncated = false;
		difficult = 0;
		valid = true;
	}
};

class Annotation2
{
public:
	std::string folder;
	std::string filename;
	std::string path;
	std::string database;
	int width;
	int height;
	int depth;
	bool segmented;
	std::vector<AnnotObject> objects;

	Annotation2()
	{
		folder = "";
		filename = "";
		path = "";
		database = "Unknown";
		width = 0;
		height = 0;
		depth = 0;
		segmented = false;
	}

	inline void Clear()
	{
		folder = "";
		filename = "";
		path = "";
		database = "Unknown";
		width = 0;
		height = 0;
		depth = 0;
		segmented = false;
		objects.clear();
	}
};

bool SaveAnnotationToXML(const Annotation2& annot, const std::string& xmlfile);

void ReadDoc(TiXmlElement* root_ele, Annotation2& annot);
void ReadObject(TiXmlNode* obj_ele, AnnotObject& obj);
void ReadFault(TiXmlNode* fault_ele, Fault& fault);

bool LoadAnnotationFromXML(Annotation2& annot, const std::string& xmlfile);


#endif
