#ifndef CITY_H
#define CITY_H

#include <iostream>
#include <sstream>

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkJPEGReader.h"
#include "vtkTexture.h"
#include "vtkPointData.h"
#include "vtkPNGReader.h"
#include "vtkJPEGReader.h"
#include "vtkPlaneSource.h"
#include "vtkTriangleFilter.h"
#include "vtkConeSource.h"
#include "vtkProperty.h"
#include "vtkCylinderSource.h "
#include "vtkTriangleStrip.h"
#include "vtkCellType.h"
#include "vtkCell.h"
#include "vtkOpenGLTexture.h"
#include "vtkSmartPointer.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"


#include "vtkCamera.h"
#include "vtkLight.h"

using namespace std;
using namespace rapidxml;

class City{

public:
	// Constructor
	City();
	~City();

	//

	// Set Actor
	void AddActor(vtkRenderer* ren);
//	void Animate(vtkRenderWindow* renwin);

	//Functions
private:
	bool isName(const xml_node<>* node, const char* name);
	bool isName(const xml_attribute<>* attr, const char* name);
	xml_node<>* getChildWithID(const xml_node<>* node, const char* id);
	const char* ReadNodeContentDirect(const xml_node<>* node);
	void readCoordinateFromLine(const char line[], vector<double> & coor);
	void readCoordinateFromLine(const char line[], vector<int> & coor);
	string findTextureName(const char * source);
	vtkActor * getActorFromModelFile(const char * name, vtkActor * newactor[],int& geo);

private:
	int geo;
	string m_sPath;
	string m_sModeFileName;
	std::vector<vtkActor*> m_vTexturedActors;
	int cellNumCounter;
	istringstream m_oIss;
};

#endif