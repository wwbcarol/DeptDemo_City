#ifndef CITY_H
#define CITY_H

#include <iostream>
#include <sstream>
#include <algorithm>

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

#include "vtkSphereSource.h"
#include "vtkTextureMapToSphere.h"
#include "vtkTransformTextureCoords.h"


#include "vtkCamera.h"
#include "vtkLight.h"

using namespace std;
using namespace rapidxml;

class City{
	class building{
	public:
		/*!
		* \brief
		* used the index in the m_vCollection as unique ID for every building
		*
		* here, ID is used to indicate what item shoud be grouped to create a
		* actor.
		*/
		int index;
		/*!
		* \brief
		* for dae file, indicate the location for the model
		*/
		double m_pdLocation[3];		
		/*!
		* \brief
		* for dae file, indicate the orientation for the model
		*/
		double m_pdOrientation[3];	
		/*!
		* \brief
		* for dae file, indicate the scale for the model
		*
		* = 100 is no changed
		* < 100 is to shrink
		* > 100 is to amplify
		*/
		double m_pdScale[3];		
		/*!
		* \brief
		* for dae file, indicate the model file path and name
		*
		* it may not be the same as the one read from kml file since it may
		* be changed according to m_sModelPath or m_sPath
		*/
		std::string m_sModelFileName;
		/*!
		* \brief
		* for simple One, keep the content of every polgon's out boundary
		*/
		std::vector<double> m_vOutBoundary;
		/*!
		* \brief
		* for simple One, keep the content of every polgon's inner boundary
		* (if there is any)
		*
		* BUT, they are not used anywhere
		*/
		std::vector<std::vector<double>> m_vvInnerBoundary;
		/*!
		* \brief
		* indicate if is building represent a model or just a simple polygon
		*/
		bool m_bIsDae;
		building(){
			m_sModelFileName = "";
			m_bIsDae = false;
			m_vvInnerBoundary.clear();
			m_vOutBoundary.clear();
		}
	};

public:
	// Constructor
	City();
	~City();

	//
	void setKMLFileName(const char * name);
	void setMapFileName(const char * name);
	void setModelPath(const char * name);
	

	void getAllBuilding(const char fileName[]);
	void findBuildingsRecursively(const xml_node<>* node);
	void getTexturedActorList(std::vector<vtkActor*>& collection);
	void getSimpleActorList(std::vector<vtkActor*>& collection);

	void loadThreeItems(const xml_node<> * node, double ray[]);
	void addBuildingFromPolygon(const xml_node<>* polygon);
	void addBuildingFromModel(const xml_node<>* model);
	void getCoordinateFromBoundary(const xml_node<> * boundary, std::vector<double> & coor);
	void createTexturedActorList();
	vtkActor * getActorFromModel(building * info, vtkActor * aa[],int& geo);
	vtkActor * getMapActor();
	vtkActor * getMapActor(const char * name, double startX, double startY, double widthX, double heightY);
	vtkActor * getBackUpActor();
	//
	void relocateXY(vtkPolyData * polyData, double * xy);
	void adjustXY(double& x, double& y);
	void adjustCoordinate(std::vector<double>& coor);
	vtkActor * getActorWithoutHole2(const std::vector<double>& coor);//trianglestrips
	void getActorWithoutHole3(vtkPolyData * polyData, const std::vector<double>& coor);
	void getActorWithoutHole3plus(vtkPolyData * polyData, const std::vector<double>& coor);
	void createSimpleActorList4();
	//inline function
	inline bool isOuterBoundryElement(const xml_node<>* node){
		return isName(node, "outerBoundaryIs");
	}
	inline bool isModelElement(const xml_node<>* node){
		return isName(node, "Model");
	}
	inline bool isInnerBoundaryElement(const xml_node<>* node){
		return isName(node, "innerBoundaryIs");
	}
	inline bool isPolygonElement(const xml_node<>* node){
		return isName(node, "Polygon");
	}
	// Set Actor
	void AddActor(vtkRenderer* ren);
//	void Animate(vtkRenderWindow* renwin);

	//Functions
private:
	void createBuildingFile();

	bool isName(const xml_node<>* node, const char* name);
	bool isName(const xml_attribute<>* attr, const char* name);
	xml_node<>* getChildWithID(const xml_node<>* node, const char* id);
	const char* ReadNodeContentDirect(const xml_node<>* node);
	void readCoordinateFromLine(const char line[], vector<double> & coor);
	void readCoordinateFromLine(const char line[], vector<int> & coor);
	string findTextureName(const char * source);
	vtkActor * getActorFromModelFile(const char * name, vtkActor * newactor[],int& geo);
	vtkActor* CreateTexturedSemiSphere(const char* texFileName, double radius, double center[3]);
private:
	int geo;
	string m_sPath;
	string m_sModelPath;
	string m_sModeFileName;

	std::string m_sKMLFileName;
	bool m_bIsNewKML;
	std::string m_sMapFileName;
	bool m_bIsNewMap;
	vtkActor * m_pMapActor;

	std::vector<vtkActor*> m_vTexturedActors;
	std::vector<building*> m_vCollection;
	std::vector<vtkActor*> m_vSimpleActors;

	int m_iCounter;
	int cellNumCounter;
	istringstream m_oIss;

	double m_dRefLatitude;
	double m_dRefLongitude;
	double m_dMapStartLongitude;
	double m_dMapStartLatitude;
	double m_dMapWidth;
	double m_dMapHeight;
	double m_dMeterPerLongitude;
	double m_dMeterPerLatitude;
};

#endif