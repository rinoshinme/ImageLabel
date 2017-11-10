#include "stdafx.h"
#include "Annotation.h"
#include "Annotation2.h"

#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"

void AddBndBox(TiXmlElement* bndbox, const cv::Rect& rect)
{
	char buf[1024];

	TiXmlElement* xmin_ele = new TiXmlElement("xmin");
	sprintf_s(buf, "%d", rect.x);
	TiXmlText* xmin_value = new TiXmlText(buf);
	xmin_ele->InsertEndChild(*xmin_value);
	bndbox->LinkEndChild(xmin_ele);

	TiXmlElement* ymin_ele = new TiXmlElement("ymin");
	sprintf_s(buf, "%d", rect.y);
	TiXmlText* ymin_value = new TiXmlText(buf);
	ymin_ele->InsertEndChild(*ymin_value);
	bndbox->LinkEndChild(ymin_ele);

	TiXmlElement* xmax_ele = new TiXmlElement("xmax");
	sprintf_s(buf, "%d", rect.x + rect.width);
	TiXmlText* xmax_value = new TiXmlText(buf);
	xmax_ele->InsertEndChild(*xmax_value);
	bndbox->LinkEndChild(xmax_ele);

	TiXmlElement* ymax_ele = new TiXmlElement("ymax");
	sprintf_s(buf, "%d", rect.y + rect.height);
	TiXmlText* ymax_value = new TiXmlText(buf);
	ymax_ele->InsertEndChild(*ymax_value);
	bndbox->LinkEndChild(ymax_ele);
}

void AddFaultInfo(TiXmlElement* fault, const std::string& name, const cv::Rect& box)
{
	// add name
	TiXmlElement* name_ele = new TiXmlElement("name");
	TiXmlText* name_value = new TiXmlText(name.c_str());
	name_ele->InsertEndChild(*name_value);
	fault->LinkEndChild(name_ele);
	// add bndbox
	TiXmlElement* bndbox_ele = new TiXmlElement("bndbox");
	AddBndBox(bndbox_ele, box);
	fault->LinkEndChild(bndbox_ele);
}

void AddObject(TiXmlElement* obj, const CAnnotation& annot)
{
	// object add name
	TiXmlElement* name_ele = new TiXmlElement("name");
	TiXmlText* name_value = new TiXmlText(annot.GetName().c_str());
	name_ele->InsertEndChild(*name_value);
	obj->LinkEndChild(name_ele);
	// object add pose
	TiXmlElement* pose_ele = new TiXmlElement("pose");
	TiXmlText* pose_value = new TiXmlText("Unspecified");
	pose_ele->InsertEndChild(*pose_value);
	obj->LinkEndChild(pose_ele);
	// object add truncated
	TiXmlElement* truncated_ele = new TiXmlElement("truncated");
	TiXmlText* truncated_value = new TiXmlText("0");
	truncated_ele->InsertEndChild(*truncated_value);
	obj->LinkEndChild(truncated_ele);
	// object add difficult
	TiXmlElement* difficult_ele = new TiXmlElement("difficult");
	TiXmlText* difficult_value = new TiXmlText("0");
	difficult_ele->InsertEndChild(*difficult_value);
	obj->LinkEndChild(difficult_ele);
	// object add bndbox
	TiXmlElement* bndbox_ele = new TiXmlElement("bndbox");
	AddBndBox(bndbox_ele, annot.GetBaseBox());
	obj->LinkEndChild(bndbox_ele);

	// object add faults
	for (int i = 0; i < annot.GetFaultNum(); ++i)
	{
		std::string fault_name = annot.GetFaultName(i);
		cv::Rect fault_box = annot.GetFaultBox(i);
		TiXmlElement* fault_ele = new TiXmlElement("fault");
		AddFaultInfo(fault_ele, fault_name, fault_box);
		obj->LinkEndChild(fault_ele);

		// AddFaultToObject(obj, fault_name, fault_rect);
	}
}

void SaveToXML(const std::vector<CAnnotation>& annotations, const std::string& srcfile, 
	const std::string& file, int width, int height, int depth)
{
	if (annotations.size() == 0)
		return;

	TiXmlDocument doc(file.c_str());
	doc.LoadFile();
	doc.Clear();

	TiXmlElement* root = new TiXmlElement("annotation");
	doc.LinkEndChild(root);

	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(srcfile.c_str(), NULL, dir, fname, ext);
	std::string folder = std::string(dir);
	std::string filename = std::string(fname) + "." + std::string(ext);
	std::string path = srcfile;

	// add folder
	TiXmlElement* folder_ele = new TiXmlElement("folder");
	TiXmlText* folder_value = new TiXmlText(folder.c_str());
	folder_ele->InsertEndChild(*folder_value);
	root->LinkEndChild(folder_ele);
	// add filename
	TiXmlElement* filename_ele = new TiXmlElement("filename");
	TiXmlText* filename_value = new TiXmlText(filename.c_str());
	filename_ele->InsertEndChild(*filename_value);
	root->LinkEndChild(filename_ele);
	// add path
	TiXmlElement* path_ele = new TiXmlElement("path");
	TiXmlText* path_value = new TiXmlText(path.c_str());
	path_ele->InsertEndChild(*path_value);
	root->LinkEndChild(path_ele);
	// add source
	TiXmlElement* source_ele = new TiXmlElement("source");
	TiXmlElement* database_ele = new TiXmlElement("database");
	TiXmlText* database_value = new TiXmlText("Unknown");
	database_ele->InsertEndChild(*database_value);
	source_ele->LinkEndChild(database_ele);
	root->LinkEndChild(source_ele);
	// add size
	char buf[1024]; // convert integer to string
	TiXmlElement* size_ele = new TiXmlElement("size");
	TiXmlElement* width_ele = new TiXmlElement("width");
	sprintf_s(buf, "%d", width);
	TiXmlText* width_value = new TiXmlText(buf);
	width_ele->InsertEndChild(*width_value);
	TiXmlElement* height_ele = new TiXmlElement("height");
	sprintf_s(buf, "%d", height);
	TiXmlText* height_value = new TiXmlText(buf);
	height_ele->InsertEndChild(*height_value);
	TiXmlElement* depth_ele = new TiXmlElement("depth");
	sprintf_s(buf, "%d", depth);
	TiXmlText* depth_value = new TiXmlText(buf);
	depth_ele->InsertEndChild(*depth_value);
	size_ele->LinkEndChild(width_ele);
	size_ele->LinkEndChild(height_ele);
	size_ele->LinkEndChild(depth_ele);
	root->LinkEndChild(size_ele);
	// add segmented
	TiXmlElement* segmented_ele = new TiXmlElement("segmented");
	TiXmlText* segmented_value = new TiXmlText("0");
	segmented_ele->InsertEndChild(*segmented_value);
	root->LinkEndChild(segmented_ele);

	// add object
	for (size_t i = 0; i < annotations.size(); ++i)
	{
		TiXmlElement* object = new TiXmlElement("object");
		AddObject(object, annotations[i]);
		root->LinkEndChild(object);
	}
	doc.SaveFile();
}

///////////////////////////////////////////////////////////////////////////////////////////////
void AddObject(TiXmlElement* obj, const AnnotObject& object)
{
	// object add name
	TiXmlElement* name_ele = new TiXmlElement("name");
	TiXmlText* name_value = new TiXmlText(object.name.c_str());
	name_ele->InsertEndChild(*name_value);
	obj->LinkEndChild(name_ele);
	// object add pose
	TiXmlElement* pose_ele = new TiXmlElement("pose");
	TiXmlText* pose_value = new TiXmlText(object.pose.c_str());
	pose_ele->InsertEndChild(*pose_value);
	obj->LinkEndChild(pose_ele);
	// object add truncated
	TiXmlElement* truncated_ele = new TiXmlElement("truncated");
	std::string truncated_str = object.truncated ? "1" : "0";
	TiXmlText* truncated_value = new TiXmlText(truncated_str.c_str());
	truncated_ele->InsertEndChild(*truncated_value);
	obj->LinkEndChild(truncated_ele);
	// object add difficult
	TiXmlElement* difficult_ele = new TiXmlElement("difficult");
	char buf[16];
	sprintf_s(buf, "%d", object.difficult);
	TiXmlText* difficult_value = new TiXmlText(buf);
	difficult_ele->InsertEndChild(*difficult_value);
	obj->LinkEndChild(difficult_ele);
	// object add bndbox
	TiXmlElement* bndbox_ele = new TiXmlElement("bndbox");
	AddBndBox(bndbox_ele, object.bndbox);
	obj->LinkEndChild(bndbox_ele);

	// object add faults
	for (size_t i = 0; i < object.faults.size(); ++i)
	{
		std::string fault_name = object.faults[i].name;
		cv::Rect fault_box = object.faults[i].bndbox;
		TiXmlElement* fault_ele = new TiXmlElement("fault");
		AddFaultInfo(fault_ele, fault_name, fault_box);
		obj->LinkEndChild(fault_ele);
	}
}

bool SaveAnnotationToXML(const Annotation2& annot, const std::string& xmlfile)
{
	TiXmlDocument doc(xmlfile.c_str());
	doc.LoadFile();
	doc.Clear();

	TiXmlElement* root = new TiXmlElement("annotation");
	doc.LinkEndChild(root);

	// add folder
	TiXmlElement* folder_ele = new TiXmlElement("folder");
	TiXmlText* folder_value = new TiXmlText(annot.folder.c_str());
	folder_ele->InsertEndChild(*folder_value);
	root->LinkEndChild(folder_ele);
	// add filename
	TiXmlElement* filename_ele = new TiXmlElement("filename");
	TiXmlText* filename_value = new TiXmlText(annot.filename.c_str());
	filename_ele->InsertEndChild(*filename_value);
	root->LinkEndChild(filename_ele);
	// add path
	TiXmlElement* path_ele = new TiXmlElement("path");
	TiXmlText* path_value = new TiXmlText(annot.path.c_str());
	path_ele->InsertEndChild(*path_value);
	root->LinkEndChild(path_ele);
	// add source
	TiXmlElement* source_ele = new TiXmlElement("source");
	TiXmlElement* database_ele = new TiXmlElement("database");
	TiXmlText* database_value = new TiXmlText(annot.database.c_str());
	database_ele->InsertEndChild(*database_value);
	source_ele->LinkEndChild(database_ele);
	root->LinkEndChild(source_ele);
	// add size
	char buf[1024]; // convert integer to string
	TiXmlElement* size_ele = new TiXmlElement("size");
	TiXmlElement* width_ele = new TiXmlElement("width");
	sprintf_s(buf, "%d", annot.width);
	TiXmlText* width_value = new TiXmlText(buf);
	width_ele->InsertEndChild(*width_value);
	TiXmlElement* height_ele = new TiXmlElement("height");
	sprintf_s(buf, "%d", annot.height);
	TiXmlText* height_value = new TiXmlText(buf);
	height_ele->InsertEndChild(*height_value);
	TiXmlElement* depth_ele = new TiXmlElement("depth");
	sprintf_s(buf, "%d", annot.depth);
	TiXmlText* depth_value = new TiXmlText(buf);
	depth_ele->InsertEndChild(*depth_value);
	size_ele->LinkEndChild(width_ele);
	size_ele->LinkEndChild(height_ele);
	size_ele->LinkEndChild(depth_ele);
	root->LinkEndChild(size_ele);
	// add segmented
	TiXmlElement* segmented_ele = new TiXmlElement("segmented");
	std::string segmented_str = annot.segmented ? "1" : "0";
	TiXmlText* segmented_value = new TiXmlText(segmented_str.c_str());
	segmented_ele->InsertEndChild(*segmented_value);
	root->LinkEndChild(segmented_ele);

	// add object
	for (size_t i = 0; i < annot.objects.size(); ++i)
	{
		TiXmlElement* object = new TiXmlElement("object");
		AddObject(object, annot.objects[i]);
		root->LinkEndChild(object);
	}
	doc.SaveFile();
	return true;
}

void ReadDoc(TiXmlElement* root_ele, Annotation2& annot)
{
	// folder
	TiXmlNode* child = root_ele->FirstChild();
	const char* folder = child->ToElement()->GetText();
	if (folder)
		annot.folder = folder;
	else
		annot.folder = "";
	// filename
	child = root_ele->IterateChildren(child);
	const char* filename = child->ToElement()->GetText();
	if (filename)
		annot.filename = filename;
	else
		annot.filename = "";
	// path
	child = root_ele->IterateChildren(child);
	const char* path = child->ToElement()->GetText();
	if (path)
		annot.path = path;
	else
		annot.path = "";
	// source-data
	child = root_ele->IterateChildren(child);
	TiXmlNode* source = child->FirstChild();
	const char* database = source->ToElement()->GetText();
	if (database)
		annot.database = database;
	else
		annot.database = "Unknown";
	// size
	child = root_ele->IterateChildren(child);
	TiXmlNode* width_ele = child->FirstChild();
	const char* width = width_ele->ToElement()->GetText();
	if (width)
		annot.width = atoi(width);
	else
		annot.width = 0;
	TiXmlNode* height_ele = child->IterateChildren(width_ele);
	const char* height = height_ele->ToElement()->GetText();
	if (height)
		annot.height = atoi(height);
	else
		annot.height = 0;
	TiXmlNode* depth_ele = child->IterateChildren(height_ele);
	const char* depth = depth_ele->ToElement()->GetText();
	if (depth)
		annot.depth = atoi(depth);
	else
		annot.depth = 0;

	// segmented
	child = root_ele->IterateChildren(child);
	const char* segmented = child->ToElement()->GetText();
	if (segmented)
		annot.segmented = ((segmented[0] == '0') ? false : true);

	// read all objects
	annot.objects.clear();
	while (child = root_ele->IterateChildren(child))
	{
		AnnotObject obj;
		ReadObject(child, obj);
		annot.objects.push_back(obj);
	}
}

void ReadObject(TiXmlNode* obj_ele, AnnotObject& obj)
{
	// name
	TiXmlNode* child = obj_ele->FirstChild();
	const char* name = child->ToElement()->GetText();
	if (name)
		obj.name = name;
	// pose
	child = obj_ele->IterateChildren(child);
	const char* pose = child->ToElement()->GetText();
	if (pose)
		obj.pose = pose;
	else
		obj.pose = "Unspecified";
	// truncated
	child = obj_ele->IterateChildren(child);
	const char* truncated = child->ToElement()->GetText();
	if (truncated)
		obj.truncated = (truncated[0] == '0' ? false : true);
	else
		obj.truncated = 0;
	// difficult
	child = obj_ele->IterateChildren(child);
	const char* difficult = child->ToElement()->GetText();
	if (difficult)
		obj.difficult = (difficult[0] == '0' ? false : true);
	else
		obj.difficult = 0;
	// bound box
	child = obj_ele->IterateChildren(child);
	TiXmlNode* xmin_ele = child->FirstChild();
	const char* xmin = xmin_ele->ToElement()->GetText();
	if (xmin)
		obj.bndbox.x = atoi(xmin);
	TiXmlNode* ymin_ele = child->IterateChildren(xmin_ele);
	const char* ymin = ymin_ele->ToElement()->GetText();
	if (ymin)
		obj.bndbox.y = atoi(ymin);
	TiXmlNode* xmax_ele = child->IterateChildren(ymin_ele);
	const char* xmax = xmax_ele->ToElement()->GetText();
	if (xmax)
		obj.bndbox.width = atoi(xmax) - obj.bndbox.x;
	TiXmlNode* ymax_ele = child->IterateChildren(xmax_ele);
	const char* ymax = ymax_ele->ToElement()->GetText();
	if (ymax)
		obj.bndbox.height = atoi(ymax) - obj.bndbox.y;

	// read faults
	obj.faults.clear();
	while (child = obj_ele->IterateChildren(child))
	{
		Fault f;
		ReadFault(child, f);
		obj.faults.push_back(f);
	}
}

void ReadFault(TiXmlNode* fault_ele, Fault& fault)
{
	// name
	TiXmlNode* child = fault_ele->FirstChild();
	const char* name = child->ToElement()->GetText();
	if (name)
		fault.name = name;
	// bndbox
	child = fault_ele->IterateChildren(child);
	TiXmlNode* xmin_ele = child->FirstChild();
	const char* xmin = xmin_ele->ToElement()->GetText();
	if (xmin)
		fault.bndbox.x = atoi(xmin);
	TiXmlNode* ymin_ele = child->IterateChildren(xmin_ele);
	const char* ymin = ymin_ele->ToElement()->GetText();
	if (ymin)
		fault.bndbox.y = atoi(ymin);
	TiXmlNode* xmax_ele = child->IterateChildren(ymin_ele);
	const char* xmax = xmax_ele->ToElement()->GetText();
	if (xmax)
		fault.bndbox.width = atoi(xmax) - fault.bndbox.x;
	TiXmlNode* ymax_ele = child->IterateChildren(xmax_ele);
	const char* ymax = ymax_ele->ToElement()->GetText();
	if (ymax)
		fault.bndbox.height = atoi(ymax) - fault.bndbox.y;
}

bool LoadAnnotationFromXML(Annotation2& annot, const std::string& filename)
{
	TiXmlDocument doc(filename.c_str());
	bool isOK = doc.LoadFile();
	if (!isOK)
		return false;

	TiXmlElement* root = doc.RootElement();
	ReadDoc(root, annot);

	return true;
}
