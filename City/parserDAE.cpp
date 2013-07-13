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

using namespace rapidxml;
using namespace std;

istringstream m_oIss;
string m_sPath = "..\\data\\";
int cellNumCounter=0;
std::vector<vtkActor*> m_vTexturedActors;

bool isName(const xml_node<>* node, const char* name){
	return strcmp(node->name(), name) == 0;
}

bool isName(const xml_attribute<>* attr, const char* name)
{
	return strcmp(attr->name(), name) == 0;
}

xml_node<>* getChildWithID(const xml_node<>* node, const char* id)
{
	string tmp = id;
	tmp.erase(0,1);
	for(xml_node<>* child = node->first_node(); NULL!=child; child = child->next_sibling())
	{
		if (child->type() == rapidxml::node_element)
		{
			xml_attribute<>* attr = child->first_attribute("id");
			if (NULL == attr)
			{
				continue;
			}
			if (strcmp(tmp.c_str(), attr->value())==0)
			{
				return child;
			}


		}


	}
	return NULL;
}

//xml_attribute<>* getAttribWithName(xml_node<>* node, const char* name)
//{
//	for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
//		if (isName(attr,(char*)name)) {
//			return attr;
//		}
//	}
//	return NULL;
//}
//
//xml_node<>* getChildWithName(const xml_node<>* parent, const char* name){
//	for (xml_node<>* child = parent->first_node(); child; child = child->next_sibling()) {
//		if (isName(child,name)) {
//			return child;
//		}
//	}
//	return NULL;
//}


const char* ReadNodeContentDirect(const xml_node<>* node)
{
	if (node == NULL || node->first_node() == NULL
		|| node->first_node()->type() != rapidxml::node_data || node->first_node()->value() == NULL) return "";
	return (const char*) node->first_node()->value();
}

void readCoordinateFromLine(const char line[], vector<double> & coor){

	m_oIss.clear();
	m_oIss.str(line);
	double tmp;
	char comma;
	while (m_oIss>>tmp) {
		coor.push_back(tmp);
	}
	return;
}
void readCoordinateFromLine(const char line[], vector<int> & coor){
	m_oIss.clear();
	m_oIss.str(line);
	int tmp;
	char comma;
	while (m_oIss>>tmp) {
		coor.push_back(tmp);
	}
	return;
}
string findTextureName(const char * source){
	ifstream input;
	char buff[1024];
	string tStr;
	//if (m_sModelPath.size()<2) {
	//	tStr = m_sPath + "textures.txt";
	//}
	//else
	tStr = m_sPath + "textures.txt";
	input.open(tStr.c_str());
	string ret;
	while (input.getline(buff,1023)) {

		int index = 0;
		for ( index = 0; index<1023; index++) {
			if (buff[index] == '>') {
				buff[index] = '\0';
				if (strcmp(source,buff) == 0) {
					index ++;
					for (; buff[index]!='\0'; index ++) {
						if (buff[index] == '<') {
							break;
						}
					}
					index ++;
					char * start = buff+index;
					int i;
					for	(i = index; buff[i] !='\0'; i++) {
						if (buff[i] == '>') {
							break;
						}
					}
					if ( buff[i] == '\0') {
						ret.append(source+1, index-1);
					}
					else
						ret.append(start,i-index);
					return ret;
				}
			}
		}
	}
	ret = source;
	ret.erase(0,1);
	return ret;
}

vtkActor * getActorFromModelFile(const char * name, vtkActor * newactor[],int& geo)
{
	cout<<"name"<<name<<endl;
	geo = 0;
	rapidxml::file<> daeFile(name);
	rapidxml::xml_document<> daeDoc;
	daeDoc.parse<0>(daeFile.data());

	rapidxml::xml_node<> *rootNode = daeDoc.first_node();
	if (NULL == rootNode) return NULL;

	xml_node<> * lib_img = rootNode->first_node("library_images");
	if(NULL == lib_img) return NULL;

	xml_node<> * lib_gem = rootNode->first_node("library_geometries");
	if(NULL == lib_gem) return NULL;

	vtkActor * actor = vtkActor::New();
	xml_node<> * image = lib_img->first_node();

	cout<<"Here!I"<<endl;
	for(xml_node<>* gemchild = lib_gem->first_node(); NULL!=gemchild; gemchild = gemchild->next_sibling())
	{
		if(gemchild->type()!= rapidxml::node_element){
			continue;
		}
		if(image->type()!=rapidxml::node_element){
			image = image->next_sibling();
		}

		xml_node<>* mesh = gemchild->first_node("mesh");
		if(NULL == mesh) return NULL;

		xml_node<>* triangles = mesh->first_node("triangles");
		if(NULL == triangles) return NULL;

		int triCount = 0;
		xml_attribute<>* triAttr = triangles->first_attribute("count");
		triCount = atoi(triAttr->value());
		//cout<< triCount << endl;

		xml_node<>* vertexNode = NULL;
		xml_node<>* tcoordnateNode = NULL;
		xml_node<>* pContent = NULL;

		for(xml_node<>* child = triangles->first_node(); NULL!=child; child = child->next_sibling())
		{
			if(child->type() != rapidxml::node_element)
			{
				continue;
			}
			if (isName(child, "input"))
			{
				xml_attribute<> * sem = child->first_attribute("semantic");
				if (!strcmp(sem->value(), "VERTEX"))
				{
					xml_attribute<>* source = child->first_attribute("source");
					vertexNode = getChildWithID(mesh, source->value());
				}
				else if (!strcmp(sem->value(), "TEXCOORD"))
				{
					xml_attribute<>* source = child->first_attribute("source");
					tcoordnateNode = getChildWithID(mesh, source->value());
				}
			}
			if (isName(child,"p"))
			{
				pContent = child;
			}
		}
		xml_node<>* positionNode = NULL;
		xml_node<>* normalNode = NULL;

		for (xml_node<>* child = vertexNode->first_node();NULL!=child;child =child->next_sibling())
		{
			if(child->type()!=rapidxml::node_element)
			{
				continue;
			}
			if (isName(child, "input"))
			{
				xml_attribute<>* sem = child->first_attribute("semantic");
				xml_attribute<>* source = child->first_attribute("source");
				if(!strcmp(sem->value(),"POSITION"))
				{
					positionNode = getChildWithID(mesh, source->value());
				}
				else
				{
					normalNode = getChildWithID(mesh, source->value());
				}
			}

		}



		int posCount = 0;
		int normalCount = 0;
		int tcoordCount = 0;

		xml_node<>* positionContent = NULL;
		xml_node<>* normalContent = NULL;
		xml_node<>* tcoordContent = NULL;

		xml_node<>* tech;
		xml_node<>* acce;

		xml_attribute<>* cnt;
		xml_attribute<>* source;

		tech = positionNode->first_node("technique_common");
		acce = tech->first_node("accessor");
		cnt = acce->first_attribute("count");
		posCount = atoi(cnt->value());
		source = acce->first_attribute("source");
		positionContent = getChildWithID(positionNode, source->value());
		cout<<"Here!II"<<endl;
		tech = normalNode->first_node("technique_common");
		acce = tech->first_node("accessor");
		cnt = acce->first_attribute("count");
		normalCount = atoi(cnt->value());
		source = acce->first_attribute("source");
		normalContent = getChildWithID(normalNode, source->value());
		cout<<"Here!III"<<endl;
		tech = tcoordnateNode->first_node("technique_common");
		acce = tech->first_node("accessor");
		cnt = acce->first_attribute("count");
		tcoordCount = atoi(cnt->value());
		source = acce->first_attribute("source");
		tcoordContent = getChildWithID(tcoordnateNode, source->value());


		cout<<"norm name: "<<normalContent->name()<<endl;
		cout<<"norm name: "<<normalContent->first_attribute()->name()<<endl;
		cout<<"norm name: "<<normalContent->type()<<endl;

		vector<double> tcoordnates;
		readCoordinateFromLine(ReadNodeContentDirect(tcoordContent),tcoordnates);
		vector<double> positions;
		readCoordinateFromLine(ReadNodeContentDirect(positionContent),positions);		
		vector<double> normals;
		readCoordinateFromLine(ReadNodeContentDirect(normalContent),normals);
		vector<int> ps;
		readCoordinateFromLine(ReadNodeContentDirect(pContent),ps);

		vtkPoints* tPoints = vtkPoints::New();
		vtkCellArray * cellArray = vtkCellArray::New();

		for (int i=0;i<ps.size();i+=2)
		{
			tPoints->InsertNextPoint(positions[(ps[i])*3], positions[(ps[i])*3+1],positions[(ps[i])*3+2]);
		}
		int index[] = {0,1,2};
		int * ind = index;

		vtkPolyData * polyData = vtkPolyData::New();

		polyData->Allocate();

		for (int i=0;i<triCount;i++)
		{
			polyData->InsertNextCell(VTK_TRIANGLE, 3, ind);
			index[0]+=3;
			index[1]+=3;
			index[2]+=3;
		}

		vtkDoubleArray * doubleArray = vtkDoubleArray::New();
		doubleArray->SetName("TCoords");
		doubleArray->SetNumberOfComponents(2);
		doubleArray->SetNumberOfTuples(triCount*3);
		for (int i = 1; i<ps.size(); i+=2) {
			double t1 = tcoordnates[ps[i]*2];
			double t2 = tcoordnates[ps[i]*2+1];
			doubleArray->SetTuple2((i-1)/2, t1,t2);
		}

		polyData->SetPoints(tPoints);
		cellNumCounter += polyData->GetNumberOfCells();
		polyData->GetPointData()->SetTCoords(doubleArray);
		vtkJPEGReader * reader = vtkJPEGReader::New();

		string tStr = name;
		int tt = 0;
		for (tt = tStr.size()-1; tt>=0; tt--) {
			if (tStr.at(tt)=='/') {
				break;
			}
		}
		if (tt>=0) {
			tStr.erase(tt,tStr.size()-tt);
			tStr.append("\\");
		}
		cout<<"STR o"<<tStr<<endl;
		xml_node<>* intFrom = image->first_node("init_from");
		string orig = "<" + string(ReadNodeContentDirect(intFrom));

		cout<<orig<<endl;
		tStr = tStr +findTextureName(orig.c_str());

		cout<<"STR"<<tStr<<endl;
		reader->SetFileName(tStr.c_str());
		reader->Update();
		cout<<reader->GetFileName()<<endl;
		vtkOpenGLTexture * texture = vtkOpenGLTexture::New();
		texture->SetInputConnection(reader->GetOutputPort());
		texture->InterpolateOn();
		vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
		mapper->SetInput(polyData);

		if (geo==0)
		{
			actor->SetMapper(mapper);
			actor->SetTexture(texture);
		}
		newactor[geo]=vtkActor::New();
		newactor[geo]->SetMapper(mapper);
		newactor[geo]->SetTexture(texture);
		image=image->next_sibling();
		tPoints->Delete();
		cellArray->Delete();
		polyData->Delete();
		mapper->Delete();
		texture->Delete();
		reader->Delete();
		geo++;
	}
	return actor;
}

int main(int argc, char** argv)
{
	m_vTexturedActors.clear();
	//	string m_sPath = "..\\data\\";
	string m_sModeFileName = m_sPath + "models/1IFC_HighLOD.dae";

	int geo = 0;
	vtkActor * newactor[13];
	vtkActor * actor = getActorFromModelFile(m_sModeFileName.c_str(), newactor, geo);
	for(int i=0;i<geo;i++)
	{
		newactor[i]->GetProperty()->SetBackfaceCulling(1);
		m_vTexturedActors.push_back(newactor[i]);

	}
	actor->GetProperty()->SetBackfaceCulling(1);
	m_vTexturedActors.push_back(actor);



	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);


	// Add the actors to the renderer, set the background and size.
	//
	for(int i=0;i<m_vTexturedActors.size();i++){
		ren->AddActor(m_vTexturedActors[i]);
	}

	ren->SetBackground(0.1, 0.2, 0.4);
	renWin->SetSize(1920, 1080);
	//	renWin->SetSize(400,200);

	//stereo
	//renWin->StereoCapableWindowOn();
	//renWin->SetStereoTypeToCrystalEyes();
	//renWin->StereoRenderOn();

	// Set up the lighting.
	//
	//vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
	//light->SetFocalPoint(1.875,0.6125,0);
	//light->SetPosition(0.875,1.6125,1);
	//ren->AddLight(light);

	// We want to eliminate perspective effects on the apparent lighting.
	// Parallel camera projection will be used. To zoom in parallel projection
	// mode, the ParallelScale is set.
	//
	ren->GetActiveCamera()->SetFocalPoint(0,0,0);
	ren->GetActiveCamera()->SetPosition(0,50,0);
	ren->GetActiveCamera()->SetViewUp(0,0,1);
	ren->GetActiveCamera()->ParallelProjectionOn();
	ren->ResetCamera();
	//	ren->GetActiveCamera()->Pitch(90);
	ren->GetActiveCamera()->SetParallelScale(1.5);

	// This starts the event loop and invokes an initial render.
	//
	vtkInteractorStyleTrackballCamera* newin = vtkInteractorStyleTrackballCamera::New();

	iren->SetInteractorStyle(newin);
	iren->Initialize();

	iren->Start();

	return EXIT_SUCCESS;
	//return 0;
}