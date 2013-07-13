#include "city.h"

City::City()
{
	cellNumCounter = 0;
	geo = 0;
	this->m_vTexturedActors.clear();
	m_sPath = "..\\data\\";
	m_sModeFileName = m_sPath + "models/1IFC_HighLOD.dae";

}

City::~City()
{
	for(std::vector<vtkActor*>::iterator it = m_vTexturedActors.begin();it!=m_vTexturedActors.end();it++){
		if((*it)!=0){
			(*it)->Delete();	
		}
	}
	this->m_vTexturedActors.clear();
}

void City::AddActor(vtkRenderer* ren)
{
	vtkActor * newactor[13];
	vtkActor * actor = getActorFromModelFile(m_sModeFileName.c_str(), newactor, geo);
	for(int i=0;i<geo;i++)
	{
		newactor[i]->GetProperty()->SetBackfaceCulling(1);
		m_vTexturedActors.push_back(newactor[i]);

	}

	actor->GetProperty()->SetBackfaceCulling(1);
	m_vTexturedActors.push_back(actor);

	for(int i=0;i<m_vTexturedActors.size();i++){
		ren->AddActor(m_vTexturedActors[i]);
	}
}

bool City::isName(const xml_node<>* node, const char* name){
	return strcmp(node->name(), name) == 0;
}

bool City::isName(const xml_attribute<>* attr, const char* name)
{
	return strcmp(attr->name(), name) == 0;
}

xml_node<>* City::getChildWithID(const xml_node<>* node, const char* id)
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

const char* City::ReadNodeContentDirect(const xml_node<>* node)
{
	if (node == NULL || node->first_node() == NULL
		|| node->first_node()->type() != rapidxml::node_data || node->first_node()->value() == NULL) return "";
	return (const char*) node->first_node()->value();
}

void City::readCoordinateFromLine(const char line[], vector<double> & coor){

	m_oIss.clear();
	m_oIss.str(line);
	double tmp;
	char comma;
	while (m_oIss>>tmp) {
		coor.push_back(tmp);
	}
	return;
}

void City::readCoordinateFromLine(const char line[], vector<int> & coor){
	m_oIss.clear();
	m_oIss.str(line);
	int tmp;
	char comma;
	while (m_oIss>>tmp) {
		coor.push_back(tmp);
	}
	return;
}

string City::findTextureName(const char * source){
	ifstream input;
	char buff[1024];
	string tStr;

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

vtkActor * City::getActorFromModelFile(const char * name, vtkActor * newactor[],int& geo)
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