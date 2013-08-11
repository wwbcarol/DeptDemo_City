#include "city.h"
const double SCENE_BACKGROUND_COLOR[] = {0.45, 0.75, 0.9} ;
const double SCENE_AMBIENT_COLOR[] = {0.7,0.7,0.7};

const double MAP_MAT_AMBIENT_DIFFUSE_COLOR[] = {0.9,0.9,0.9};
const double MAP_MAT_AMBIENT_COEF = 1.0;
const double MAP_MAT_DIFFUSE_COEF = 1.0;
const double BUILDING_MAT_AMBIENT_DIFFUSE_COLOR[] = {0.9,0.9,0.9} ;
const double BUILDING_MAT_SPECULAR_COLOR[] = {0.0,0.0,0.0} ;
const double BUILDING_MAT_AMBIENT_COEF = 0.4 ;
const double BUILDING_MAT_DIFFUSE_COEF = 0.9 ;
const bool BUILDING_BACKFACE_CULLING = false ;
const double CLIP_PLANE_3D_FAR = 50000.0 ;
const double CLIP_PLANE_3D_NEAR = 1.0  ;
const double CLIP_PLANE_BIRD_FAR = 50000.0 ;
const double CLIP_PLANE_BIRD_NEAR = 1000.0 ;

City::City()
{

	cout<<"constructor"<<endl;

	cellNumCounter = 0;
	m_iCounter = 0;
	m_dMapHeight = 0.05;
	m_dMapWidth = 0.054;
	m_dMapStartLongitude = 114.14578;
	m_dMapStartLatitude = 22.26006;
	m_dMeterPerLatitude = 22.26;
	m_dRefLongitude = 114.1;
	m_dRefLatitude = 22.2;
	m_dMeterPerLongitude = 102893.3;
	m_dMeterPerLatitude = 111194.9;
	m_sModelPath = "";
	m_sKMLFileName = "";
	m_vCollection.clear();
	m_vTexturedActors.clear();
	m_vSimpleActors.clear();


	m_bIsNewKML = false;

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

vtkActor* City::CreateTexturedSemiSphere(const char* texFileName, double radius, double center[3])
{
	vtkSphereSource* sphere = vtkSphereSource::New();
	sphere->SetPhiResolution(50);
	sphere->SetThetaResolution(50);
	sphere->SetEndPhi(180);
	sphere->SetRadius(radius);
	sphere->SetCenter(center);

	vtkTextureMapToSphere* tmap = vtkTextureMapToSphere::New();
	tmap->PreventSeamOn();
	tmap->SetInputConnection(sphere->GetOutputPort());

	vtkTransformTextureCoords* transTex = vtkTransformTextureCoords::New();	
	transTex->SetInput(tmap->GetOutput());

	vtkPolyDataMapper* sphereMapper = vtkPolyDataMapper::New();
	sphereMapper->SetInputConnection(transTex->GetOutputPort());


	vtkActor* sphereActor = vtkActor::New();
	sphereActor->SetMapper(sphereMapper);

	vtkJPEGReader* reader = vtkJPEGReader::New();

	std::string tmp = texFileName;
	if (tmp.at(tmp.size()-3) == 'j'&&
		tmp.at(tmp.size()-2) == 'p'&&
		tmp.at(tmp.size()-1) == 'g')
	{
		reader->SetFileName(texFileName);
	}
	else
	{
		cout<<"read the sphere texture map error"<<endl;
		return NULL;
	}
	reader->Update();
	vtkTexture * texture = vtkTexture::New();
	texture->SetInputConnection(reader->GetOutputPort());
	texture->InterpolateOn();	
	sphereActor->SetTexture(texture);

	sphereActor->GetProperty()->SetDiffuseColor(0.9, 0.9, 0.9);
	sphereActor->GetProperty()->SetAmbientColor(0.9, 0.9, 0.9);
	sphereActor->GetProperty()->SetDiffuse(1.0);
	sphereActor->GetProperty()->SetAmbient(1.0);
	sphereActor->GetProperty()->SetSpecular(0.0);

	//deleting
	sphere->Delete();
	tmap->Delete();
	transTex->Delete();
	sphereMapper->Delete();
	reader->Delete();
	texture->Delete();

	return sphereActor ;
}

void City::AddActor(vtkRenderer* ren)
{

	this->setKMLFileName("..\\mapModel\\HK3D_model\\doc.kml");
	this->setMapFileName("..\\mapModel\\map.png");

	// this actor is for the 2d map
	vtkActor * actor = this->getMapActor();
	ren->SetBackground( (double*)SCENE_BACKGROUND_COLOR );
	ren->SetAmbient( (double*)SCENE_AMBIENT_COLOR );

	//set the property of 2d map
	actor->GetProperty()->SetDiffuseColor(0.9, 0.9, 0.9);
	actor->GetProperty()->SetAmbientColor(0.9, 0.9, 0.9);
	actor->GetProperty()->SetDiffuse(1.0);
	actor->GetProperty()->SetAmbient(1.0);
	actor->GetProperty()->SetSpecular(0.0);
	ren->AddActor( actor );

	//add the sky sphere
	double radius;
	double *center;
	center = actor->GetCenter();
	radius = center[0];
	cout<<"sphere center:"<<center[0]<<" "<<center[1]<<" "<<center[2]<<endl;
	ren->AddActor(CreateTexturedSemiSphere("..\\mapModel\\clouds.jpg",radius,center));

	//this actors is for buildings over the map
	std::vector<vtkActor*> actors;
	actors.clear();
	this->getTexturedActorList(actors);
	int index_EndOfTectureBuilding = actors.size();
	//this->getSimpleActorList(actors);
	int num_buildings = actors.size();

	// Height
	cout<<"num_of_buildings:"<<actors.size()<<endl;

	float* buildingHeight = new float[num_buildings];

	

	int i;
	float highest = 0.0;
	for(i=0;i<actors.size();i++){
		double* center;
		vtkActor* current_building = actors[i];
		center = current_building->GetCenter();
		buildingHeight[i] = (float) center[2];

		if (highest<buildingHeight[i])
		{
			highest = buildingHeight[i];
		}
	}
	cout<< "Highest building is"<< highest << endl;


	for (i=0;i<actors.size();i++)
	{
		vtkActor* tmp = actors[i];

		float scale = buildingHeight[i]/highest;

		// set of building properties here...
		tmp->GetProperty()->SetDiffuseColor( (double*)BUILDING_MAT_AMBIENT_DIFFUSE_COLOR );
		tmp->GetProperty()->SetAmbientColor( (double*)BUILDING_MAT_AMBIENT_DIFFUSE_COLOR) ;
		tmp->GetProperty()->SetSpecularColor( (double*) BUILDING_MAT_SPECULAR_COLOR );
		tmp->GetProperty()->SetAmbient(BUILDING_MAT_AMBIENT_COEF );
		tmp->GetProperty()->SetDiffuse( (double) scale );
		//tmp->GetProperty()->SetDiffuse(BUILDING_MAT_DIFFUSE_COEF);
		tmp->GetProperty()->SetBackfaceCulling( BUILDING_BACKFACE_CULLING );
		tmp->GetProperty()->SetInterpolationToFlat();
		// tmp->GetProperty()->SetOpacity(0.5); // test code for transparency

		if (i<index_EndOfTectureBuilding) // is the building with textures
		{
			tmp->GetProperty()->SetAmbient(0.7);
			tmp->GetProperty()->SetDiffuse(BUILDING_MAT_DIFFUSE_COEF);
		}

		ren->AddActor(tmp);
	}



	delete []buildingHeight;



	//add new light
	//ren->AddLight(CreateSceneLight());
	//ren->AddLight(CreateCameraLight());

	// Camera initial settting...
	ren->GetActiveCamera()->SetViewUp(0,1,0); // define the camera up vector
	ren->GetActiveCamera()->SetClippingRange(0.1,500); // set nearest and farest distance can be viewed
	ren->GetActiveCamera()->SetPosition(center[0]-250,center[1], radius/2.0);
	ren->GetActiveCamera()->SetFocalPoint(center);

	ren->GetActiveCamera()->SetViewUp(0,0,1); // define the camera up vector

	// set nearest and farest distance can be viewed in 3D view
	ren->GetActiveCamera()->SetClippingRange(CLIP_PLANE_3D_NEAR ,CLIP_PLANE_3D_FAR ); 

	// set nearest and farest distance can be viewed in bird view
	ren->GetActiveCamera()->SetClippingRange(CLIP_PLANE_BIRD_NEAR,CLIP_PLANE_BIRD_FAR ); 
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

	if (m_sModelPath.size()<2) {
		tStr = m_sPath + "textures.txt";
	}
	else
		tStr = m_sModelPath + "textures.txt";

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

		tech = normalNode->first_node("technique_common");
		acce = tech->first_node("accessor");
		cnt = acce->first_attribute("count");
		normalCount = atoi(cnt->value());
		source = acce->first_attribute("source");
		normalContent = getChildWithID(normalNode, source->value());

		tech = tcoordnateNode->first_node("technique_common");
		acce = tech->first_node("accessor");
		cnt = acce->first_attribute("count");
		tcoordCount = atoi(cnt->value());
		source = acce->first_attribute("source");
		tcoordContent = getChildWithID(tcoordnateNode, source->value());

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

		xml_node<>* intFrom = image->first_node("init_from");
		string orig = "<" + string(ReadNodeContentDirect(intFrom));

		tStr = tStr +findTextureName(orig.c_str());

		reader->SetFileName(tStr.c_str());
		reader->Update();

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

void City::setKMLFileName(const char * name){
	m_sKMLFileName = name;
	m_bIsNewKML = true;
}

void City::setModelPath(const char * name){
	m_sModelPath = name;
	if (m_sModelPath.at(m_sModelPath.size()-1) != '\\' ||
		m_sModelPath.at(m_sModelPath.size()-1) != '/') {
			m_sModelPath += "\\";
	}
}

void City::setMapFileName(const char * name){
	m_sMapFileName = name;
	m_bIsNewMap = true;
}

void City::getAllBuilding(const char fileName[]){
	//here we first collect all the information to m_vCollection
	//for dae file, we collection 
	m_vCollection.clear();
	string tmp = fileName;
	cout<<"(f)getAllBuilding, fileName :" << fileName<<endl;
	int index = 0;
	for (index = tmp.size()-1; index>=0; index--) {
		if (tmp.at(index)=='\\') {
			break;
		}
	}
	if (index>=0) {
		tmp.erase(index+1,tmp.size()-index-1);
	}
	m_sPath = tmp;

	cout<<"\tm_sPath :" << m_sPath << endl;

	rapidxml::file<> kmlFile(fileName);
	rapidxml::xml_document<> kmlDoc;
	kmlDoc.parse<0>(kmlFile.data());

	rapidxml::xml_node<> *rootNode = kmlDoc.first_node();

	findBuildingsRecursively(rootNode);

	createBuildingFile();
	createTexturedActorList();
	createSimpleActorList4();

	cout<<"End of findAll" << endl;
	int number = 0;
	number += m_vSimpleActors.size();
	number += m_vTexturedActors.size();
	cout<<"[info] Building actors generated: "<<number<<endl;
	//createSimpleActorList2();
}
void City::createSimpleActorList4()
{
	if (m_vSimpleActors.size() > 0)
	{
		for (int i = 0; i<m_vSimpleActors.size(); i++)
		{
			m_vSimpleActors[i]->Delete();
		}
		m_vSimpleActors.clear();
	}
	ifstream input("..\\programData\\building.txt");
	istringstream iss;
	iss.clear();
	char buff[1024];
	int * marks = new int[6000];
	for (int i = 0; i < 6000; i++)
	{
		marks[i] = 0;
	}
	char c;
	while (input.getline(buff,1023))
	{
		vector<int> slotIndex;
		iss.clear();
		iss.str(buff);
		int temp;
		iss>>temp;
		if (marks[temp] == 1)
		{
			cout<<"ERROR: actor already created(0)"<<endl;
		}
		marks[temp] = 1;
		vtkPolyData * polyData = vtkPolyData::New();
		adjustCoordinate(m_vCollection[temp]->m_vOutBoundary);
		getActorWithoutHole3plus(polyData, m_vCollection[temp]->m_vOutBoundary);//3plus
		slotIndex.push_back(m_vCollection[temp]->m_vOutBoundary.size() / 3 * 2);
		iss>>c;
		while (iss>>temp)
		{
			if (marks[temp] == 1)
			{
				cout<<"ERROR: actor already created(1)"<<endl;
			}
			marks[temp] = 1;
			adjustCoordinate(m_vCollection[temp]->m_vOutBoundary);
			getActorWithoutHole3(polyData,m_vCollection[temp]->m_vOutBoundary);//3
			slotIndex.push_back(m_vCollection[temp]->m_vOutBoundary.size() / 3 * 2);
		}
		double org[3];
		org[2] = 0;
		relocateXY(polyData, org);

		////new by Hong
		//MyvtkOpenGLTexture* atext=NULL;
		//if (ADD_TEXTURE_TO_SIMPLE_BUILDING)
		//{
		//	atext = setTCoordForSimpleBuilding(polyData, slotIndex);
		//}
		vtkPolyDataMapper * polyMapper = vtkPolyDataMapper::New();
		polyMapper->SetInput(polyData);

		vtkActor * actor = vtkActor::New();
		actor->SetMapper(polyMapper);
		actor->SetPosition(org);
		//if (ADD_TEXTURE_TO_SIMPLE_BUILDING)
		//{
		//	actor->SetTexture(atext);
		//	atext->Delete();
		//}
		////end of new by Hong

		m_vSimpleActors.push_back(actor);
	}
	for (int i = 0; i< 6000; i++)
	{
		if (i == m_vCollection.size())
		{
			break;
		}
		if (0 != marks[i] || m_vCollection[i]->m_bIsDae)
		{
			continue;
		}
		vtkActor * tmp = NULL;
		adjustCoordinate(m_vCollection[i]->m_vOutBoundary);
		tmp = getActorWithoutHole2(m_vCollection[i]->m_vOutBoundary);//2
		if (NULL != tmp)
		{
			m_vSimpleActors.push_back(tmp);
		}
	}
	delete[] marks;
}

void City::relocateXY(vtkPolyData * polyData, double * xy){
	//change to local coordinate and actor position style
	//which is what textured building is while simple building not
	vtkPoints * tPoints = polyData->GetPoints();
	int size = tPoints->GetNumberOfPoints();
	double point[3];
	double bounds[6];
	tPoints->GetBounds(bounds);

	double midX = bounds[0] *0.5 + bounds[1] * 0.5;
	double midY = bounds[2] *0.5 + bounds[3] * 0.5;
	for (int i = 0; i<size; i++)
	{
		tPoints->GetPoint(i,point);
		point[0] -= midX;
		point[1] -= midY;
		tPoints->SetPoint(i,point);
	}
	xy[0] = midX;
	xy[1] = midY;
}

vtkActor * City::getActorWithoutHole2(const vector<double>& coor){
	vector<int> slotIndex;
	slotIndex.push_back(coor.size()/3 * 2);
	static vtkTriangleFilter * tri = vtkTriangleFilter::New();
	vtkPolyData * tPD = vtkPolyData::New();
	int topPoint = coor.size()/3;
	vtkPoints * tPoints = vtkPoints::New();
	for (int i = 0; i< topPoint; i++){
		tPoints->InsertNextPoint(coor[i*3], coor[i*3+1], 0);
		tPoints->InsertNextPoint(coor[i*3], coor[i*3+1], coor[i*3+2]);
	}
	vtkTriangleStrip * triangleStrip = vtkTriangleStrip::New();
	triangleStrip->GetPointIds()->SetNumberOfIds(2*topPoint);
	for (int i = 0; i<topPoint*2; i++)
	{
		triangleStrip->GetPointIds()->SetId(i,i);
	}

	vtkCellArray * cellArray = vtkCellArray::New();
	cellArray->InsertNextCell(topPoint-1);
	for (int i = 0; i<topPoint - 1; i++)
	{
		cellArray->InsertCellPoint(i*2+1);
	}
	tPD->SetPoints(tPoints);
	tPD->SetPolys(cellArray);

	tri->SetGlobalWarningDisplay(0);
	tri->SetInput(tPD);
	tPD = tri->GetOutput();
	tPD->Update();
	cellNumCounter += tPD->GetNumberOfCells();

	int cellCount = tPD->GetNumberOfCells();
	cellNumCounter += cellCount;
	cellNumCounter ++;
	vtkPolyData * polyData = vtkPolyData::New();
	polyData->SetPoints(tPoints);
	polyData->Allocate();
	polyData->InsertNextCell(triangleStrip->GetCellType(),triangleStrip->GetPointIds());

	for (int i = 0; i<cellCount; i++)
	{
		polyData->InsertNextCell(VTK_TRIANGLE, tPD->GetCell(i)->GetPointIds());
	}
	double pos[3];
	pos[2] = 0;
	relocateXY(polyData, pos);

	////*********************************************************************************************************~~~~
	////Add texture to the buildings
	//MyvtkOpenGLTexture* atext=NULL;
	//if (ADD_TEXTURE_TO_SIMPLE_BUILDING)
	//{
	//	atext = setTCoordForSimpleBuilding(polyData, slotIndex);
	//}
	vtkPolyDataMapper * polyMapper = vtkPolyDataMapper::New();
	polyMapper->SetInput(polyData);

	vtkActor * actor = vtkActor::New();
	actor->SetMapper(polyMapper);
	actor->SetPosition(pos);
	//if (ADD_TEXTURE_TO_SIMPLE_BUILDING)
	//{
	//	actor->SetTexture(atext);
	//	atext->Delete();
	//}
	////end 

	tPoints->Delete();
	cellArray->Delete();
	polyData->Delete();
	polyMapper->Delete();

	return actor;
}

void City::getActorWithoutHole3(vtkPolyData * polyData, const std::vector<double>& coor){
	int bias =polyData->GetPoints()->GetNumberOfPoints();
	vtkTriangleFilter * tri = vtkTriangleFilter::New();
	vtkPolyData * tPD = vtkPolyData::New();
	int topPoint = coor.size()/3;
	for (int i = 0; i< topPoint; i++){
		polyData->GetPoints()->InsertPoint(bias+2*i,coor[i*3], coor[i*3+1], 0);
		polyData->GetPoints()->InsertPoint(bias+2*i+1,coor[i*3], coor[i*3+1], coor[i*3+2]);
	}
	vtkTriangleStrip * triangleStrip = vtkTriangleStrip::New();
	triangleStrip->GetPointIds()->SetNumberOfIds(2*topPoint);
	for (int i = 0; i<topPoint*2; i++)
	{
		triangleStrip->GetPointIds()->SetId(i,i+bias);
	}

	vtkCellArray * cellArray = vtkCellArray::New();
	cellArray->InsertNextCell(topPoint-1);
	for (int i = 0; i<topPoint - 1; i++)
	{
		cellArray->InsertCellPoint(i*2+1+bias);
	}
	tPD->SetPoints(polyData->GetPoints());
	tPD->SetPolys(cellArray);

	tri->SetGlobalWarningDisplay(0);
	tri->SetInput(tPD);
	tPD = tri->GetOutput();
	tPD->Update();
	cellNumCounter += tPD->GetNumberOfCells();

	int cellCount = tPD->GetNumberOfCells();
	cellNumCounter += cellCount;
	cellNumCounter ++;
	polyData->InsertNextCell(triangleStrip->GetCellType(),triangleStrip->GetPointIds());

	for (int i = 0; i<cellCount; i++)
	{
		polyData->InsertNextCell(VTK_TRIANGLE, tPD->GetCell(i)->GetPointIds());
	}
}

void City::getActorWithoutHole3plus(vtkPolyData * polyData, const std::vector<double>& coor){
	vtkTriangleFilter * tri = vtkTriangleFilter::New();
	vtkPolyData * tPD = vtkPolyData::New();
	int topPoint = coor.size()/3;
	vtkPoints * tPoints = vtkPoints::New();
	for (int i = 0; i< topPoint; i++){
		tPoints->InsertNextPoint(coor[i*3], coor[i*3+1], 0);
		tPoints->InsertNextPoint(coor[i*3], coor[i*3+1], coor[i*3+2]);
	}
	vtkTriangleStrip * triangleStrip = vtkTriangleStrip::New();
	triangleStrip->GetPointIds()->SetNumberOfIds(2*topPoint);
	for (int i = 0; i<topPoint*2; i++)
	{
		triangleStrip->GetPointIds()->SetId(i,i);
	}

	vtkCellArray * cellArray = vtkCellArray::New();
	cellArray->InsertNextCell(topPoint-1);
	for (int i = 0; i<topPoint - 1; i++)
	{
		cellArray->InsertCellPoint(i*2+1);
	}
	tPD->SetPoints(tPoints);
	tPD->SetPolys(cellArray);

	tri->SetGlobalWarningDisplay(0);
	tri->SetInput(tPD);
	tPD = tri->GetOutput();
	tPD->Update();
	cellNumCounter += tPD->GetNumberOfCells();

	int cellCount = tPD->GetNumberOfCells();
	cellNumCounter += cellCount;
	cellNumCounter ++;
	polyData->SetPoints(tPoints);
	polyData->Allocate();
	polyData->InsertNextCell(triangleStrip->GetCellType(),triangleStrip->GetPointIds());

	for (int i = 0; i<cellCount; i++)
	{
		polyData->InsertNextCell(VTK_TRIANGLE, tPD->GetCell(i)->GetPointIds());
	}
}

void City::adjustCoordinate(vector<double>& coor){
	int num = coor.size();
	for (int i = 0; i<num; i++) {
		if (i % 3 == 0) {
			adjustXY(coor[i], coor[i+1]);
		}
	}
}
void City::createTexturedActorList(){
	if (m_vTexturedActors.size() > 0)
	{
		for (int i = 0; i<m_vTexturedActors.size(); i++)
		{
			m_vTexturedActors[i]->Delete();
		}
		m_vTexturedActors.clear();
	}
	for (int i = 0; i< m_vCollection.size(); i++) {
		vtkActor * tmp = NULL;
		if (m_vCollection[i]->m_bIsDae) {
			if (m_vCollection[i]->m_pdLocation[0]<114) {
				continue;
			}
			vtkActor * temp[13];
			int geo=0;
			tmp = getActorFromModel(m_vCollection[i],temp,geo);
			if (NULL == tmp) {
				continue;
			}
			for (int i=0; i<geo; i++){
				temp[i]->GetProperty()->SetBackfaceCulling(1);
				m_vTexturedActors.push_back(temp[i]);
			}	
			tmp->GetProperty()->SetBackfaceCulling(1);
			m_vTexturedActors.push_back(tmp);

		}
	}

}

void City::adjustXY(double& x, double& y){
	//static const double xPerDegree = 102893.3;//in meters
	//static const double yPerDegree = 111194.9;//in meters
	//static const double HKx        = 114.1;//in degrees
	//static const double HKy        = 22.2; //in degrees
	x = (x - m_dRefLongitude) * m_dMeterPerLongitude;
	y = (y - m_dRefLatitude) * m_dMeterPerLatitude;

}

vtkActor * City::getMapActor(const char * name, double startX, double startY, double widthX, double heightY){
	vector<double> plate;
	plate.push_back(startX);
	plate.push_back(startY);
	plate.push_back(1);
	plate.push_back(startX+widthX);
	plate.push_back(startY);
	plate.push_back(1);
	plate.push_back(startX+widthX);
	plate.push_back(startY+heightY);
	plate.push_back(1);
	plate.push_back(startX);
	plate.push_back(startY+heightY);
	plate.push_back(1);
	adjustCoordinate(plate);

	string tmp = name;
	vtkPNGReader * pngReader = vtkPNGReader::New();;
	vtkJPEGReader *jpegReader= vtkJPEGReader::New();

	vtkTexture * atext = vtkTexture::New();
	//MyvtkOpenGLTexture* atext = MyvtkOpenGLTexture::New();

	if (tmp.at(tmp.size()-3) == 'p'&&
		tmp.at(tmp.size()-2) == 'n'&&
		tmp.at(tmp.size()-1) == 'g') {
			pngReader->SetFileName(name);
			atext->SetInputConnection(pngReader->GetOutputPort());
	}
	else if (tmp.at(tmp.size()-3) == 'j'&&
		tmp.at(tmp.size()-2) == 'p'&&
		tmp.at(tmp.size()-1) == 'g'){
			jpegReader->SetFileName(name);
			atext->SetInputConnection(jpegReader->GetOutputPort());
	}
	else{
		pngReader->Delete();
		jpegReader->Delete();
		return NULL;
	}
	atext->SetQualityTo32Bit();
	atext->InterpolateOn();
	vtkPlaneSource * plane = vtkPlaneSource::New();
	plane->SetOrigin(plate[0],plate[1],0);
	plane->SetXResolution(32);
	plane->SetYResolution(32);
	plane->SetPoint1(plate[3],plate[4],0);
	plane->SetPoint2(plate[9],plate[10],0);
	vtkPolyDataMapper * planeMapper = vtkPolyDataMapper::New();
	planeMapper->SetInputConnection(plane->GetOutputPort());
	vtkActor * planeActor = vtkActor::New();
	planeActor->SetMapper(planeMapper);
	planeActor->SetTexture(atext);

	pngReader->Delete();
	jpegReader->Delete();
	atext->Delete();
	planeMapper->Delete();
	plane->Delete();

	return planeActor;
}

vtkActor * City::getMapActor(){
	if (m_bIsNewMap)
	{
		if (m_sMapFileName.size()<2) {
			return getBackUpActor();
		}
		m_pMapActor = getMapActor
			(m_sMapFileName.c_str(),
			m_dMapStartLongitude,
			m_dMapStartLatitude,
			m_dMapWidth,
			m_dMapHeight);
		m_bIsNewMap = false;
	}
	if (NULL != m_pMapActor)
	{
		return m_pMapActor;
	}
	else
	{
		return getBackUpActor();
	}
}

vtkActor * City::getBackUpActor(){
	vtkConeSource *cone = vtkConeSource::New();
	cone->SetHeight( 3.0 );
	cone->SetRadius( 1.0 );
	cone->SetResolution( 10 );

	vtkPolyDataMapper *coneMapper = vtkPolyDataMapper::New();
	coneMapper->SetInputConnection( cone->GetOutputPort() );

	vtkActor *coneActor = vtkActor::New();
	coneActor->SetMapper( coneMapper );
	return coneActor;
}

vtkActor * City::getActorFromModel(building * info,vtkActor * newactor[],int& geo){
	geo=1;
	//cout<<"T"<<endl;
	vtkActor * actor = getActorFromModelFile(info->m_sModelFileName.c_str(),newactor,geo);
	if (NULL == actor) {
		cout<<"ERROR: Reading model"<<endl;
	}
	adjustXY(info->m_pdLocation[0], info->m_pdLocation[1]);
	actor->SetPosition(info->m_pdLocation[0],info->m_pdLocation[1],info->m_pdLocation[2]);
	actor->SetScale(info->m_pdScale[0]/100.0, info->m_pdScale[1]/100.0, info->m_pdScale[2]/100.0);
	double rotateZ = info->m_pdOrientation[0];
	if (rotateZ>0) {
		rotateZ = 360-rotateZ;
	}
	else
		rotateZ = 0-rotateZ;
	actor->RotateWXYZ(rotateZ,0,0,1);
	for (int i=1; i<geo; i++){
		//	cout<<"GAFM "<<i<<endl;
		//newactor[i]->SetPosition(info->m_pdLocation[0],info->m_pdLocation[1],0);
		//newactor[i]->SetScale(info->m_pdScale[0]/100.0, info->m_pdScale[1]/100.0, info->m_pdScale[2]/100.0);
		newactor[i]->SetPosition(info->m_pdLocation[0],info->m_pdLocation[1],info->m_pdLocation[2]);
		newactor[i]->SetScale(info->m_pdScale[0]/100.0, info->m_pdScale[1]/100.0, info->m_pdScale[2]/100.0);
		newactor[i]->RotateWXYZ(rotateZ,0,0,1);
	}

	return actor;
}

void City::findBuildingsRecursively(const xml_node<>* node){
	for (xml_node<>* child = node->first_node(); NULL != child; child = child->next_sibling()) {
		if (child->type() != rapidxml::node_element) {
			continue;
		}
		if (isPolygonElement(child)) {
			addBuildingFromPolygon(child);
		}
		if (isModelElement(child)) {
//			cout<<"(f)Recursive: child:"<<child->name()<<"isModelElement"<<endl;
			addBuildingFromModel(child);
		}

	}
	for (xml_node<>* child = node->first_node(); child != NULL; child = child->next_sibling()) {
		if (child->type() != rapidxml::node_element) {
			continue;
		}
		findBuildingsRecursively(child);
	}
}

void City::addBuildingFromPolygon(const xml_node<>* polygon){
	building * toAdd = new building;
	xml_node<> * outerBoundary = NULL;
	for (xml_node<> * child = polygon->first_node(); NULL != child; child = child->next_sibling()) 
	{
		if (isOuterBoundryElement(child)) {
			outerBoundary = child;
		}
		if (isInnerBoundaryElement(child)) {
			vector<double> tmp;
			getCoordinateFromBoundary(child,tmp);
			if (tmp.size() > 0) {
				toAdd->m_vvInnerBoundary.push_back(tmp);
			}
		}
	}
	if ( NULL == outerBoundary) {
		delete toAdd;
		return;
	}
	getCoordinateFromBoundary(outerBoundary,toAdd->m_vOutBoundary);
	toAdd->index = m_iCounter;
	m_iCounter ++;
	m_vCollection.push_back(toAdd);
}

void City::getCoordinateFromBoundary(const xml_node<> * boundary, vector<double> & coor){
	xml_node<> * tNode = boundary->first_node("LinearRing");
	if ( NULL != tNode) {
		tNode = tNode->first_node("coordinates");
	}
	if ( NULL != tNode) {
		m_oIss.clear();
		m_oIss.str(ReadNodeContentDirect(tNode));
		double t;
		char comma;
		while (m_oIss>>t) {
			coor.push_back(t);
			m_oIss>>comma;
			if (comma != ',') {
				cout<<"ERROR when read the coordinate: missing comma"<<endl;
			}
		}
	}
}

void City::addBuildingFromModel(const xml_node<>* model){
	building * toAdd = new building;
	int check = 0;
	for (xml_node<>* child = model->first_node(); NULL != child; child = child->next_sibling()) {
		if (child->type() != rapidxml::node_element) {
			continue;
		}
		if (isName(child,"Location")) {
			loadThreeItems(child,toAdd->m_pdLocation);
			check ++;
		}
		else if (isName(child,"Orientation")) {
			loadThreeItems(child,toAdd->m_pdOrientation);
			check ++;
		}
		else if (isName(child,"Scale")) {
			loadThreeItems(child,toAdd->m_pdScale);
			check ++;
		}
		else if (isName(child,"Link")) {
			xml_node<> * tmp = child->first_node("href");
			if (NULL == tmp) {
				delete toAdd;
				return;
			}
			string modelFileName = ReadNodeContentDirect(tmp);
			if (m_sModelPath.size()<2) {
				toAdd->m_sModelFileName = m_sPath + modelFileName;
			}
			else{
				toAdd->m_sModelFileName = m_sModelPath + modelFileName;
			}
			check ++;
			toAdd->m_bIsDae = true;
		}
		if (check == 4) {
			break;
		}
	}
	if (check != 4) {
		cout<<"ERROR: dropping one model"<<endl;
		delete toAdd;
		return;
	}
	toAdd->index = m_iCounter;
	m_iCounter ++;
	m_vCollection.push_back(toAdd);
}

void City::loadThreeItems(const xml_node<> * node, double ray[]){
	int index = 0;
	for (xml_node<>* child = node->first_node(); NULL != child && index < 3; child = child->next_sibling()) {
		if (child->type() != rapidxml::node_element) {
			continue;
		}
		ray[index] = atof(ReadNodeContentDirect(child));
		index ++;
	}
	if (index != 3) {
		cout<<"ERROR when reading triples~~"<<endl;
	}
}

void City::getTexturedActorList(vector<vtkActor*>& collection){

	if(m_bIsNewKML == true)
	cout<<"\n(f)getTexturedActorList:\nm_bIsNewKML:true\n";
	else
	cout<<"\n(f)getTexturedActorList:\nm_bIsNewKML:false\n";

	if (m_bIsNewKML)
	{
		cout<<"\ncall (f)getAllBuilding\n";
		getAllBuilding(m_sKMLFileName.c_str());
		cout<<"\n(f)getTexturedActorList: End of getAllBuilding\n";
		m_bIsNewKML = false;
	}
	cout<<"\n(f)getTexturedActorList:\nm_vTexturedActors.size:"<<m_vTexturedActors.size();
	for (int i = 0; i<m_vTexturedActors.size(); i++)
	{
		collection.push_back(m_vTexturedActors[i]);
	}
	cout<<"End of getTexturedActorList"<<endl;
}

void City::getSimpleActorList(vector<vtkActor*>& collection){
	if (m_bIsNewKML)
	{
		getAllBuilding(m_sKMLFileName.c_str());
		m_bIsNewKML = false;
	}
	for (int i = 0; i<m_vSimpleActors.size(); i++)
	{
		collection.push_back(m_vSimpleActors[i]);
	}


	//if (HACK_TO_REMOVE_BUILDING)
	//{
	//	this->HackToRemoveBuildings(collection);
	//}

}
void City::createBuildingFile(){
	ofstream output("..\\programData\\building_new.txt");
	int sStart;//find the first Simple element
	for (int i = 0; i< m_vCollection.size(); i++)
	{
		if (m_vCollection[i]->m_bIsDae == false)
		{
			sStart = i;
			break;
		}
	}
	{//sorting the m_vCollection
		building * b = NULL;
		for (int i = sStart; i<m_vCollection.size(); i++){
			b = m_vCollection[i];
			int j;
			for (j = i-1; j>=sStart; j--){
				if (m_vCollection[j]->m_vOutBoundary[0] > b->m_vOutBoundary[0]){
					m_vCollection[j+1] = m_vCollection[j];
				}
				else{
					break;
				}
			}		
			m_vCollection[j+1] = b;
		}
	}//end of sorting
	///////////////////////////////////////////////////////////////////////////
	int * mark = new int[m_vCollection.size()];
	for (int i = 0; i< m_vCollection.size(); i++)
	{
		mark[i] = 0;
	}

	{
		for (int i = sStart; i<m_vCollection.size(); i++)
		{
			building * b = m_vCollection[i];
			if (b->m_vvInnerBoundary.size() > 0)//if some building has hole
			{
				int start = sStart;
				for (int j = i; j>=sStart; j--)
				{
					if (b->m_vOutBoundary[0] - m_vCollection[j]->m_vOutBoundary[0] > 0.004)
					{
						start = j;
						break;
					}
				}
				int end = m_vCollection.size()-1;
				for (int j = i; j<m_vCollection.size(); j++)
				{
					if (m_vCollection[j]->m_vOutBoundary[0] - b->m_vOutBoundary[0] > 0.004)
					{
						end = j;
						break;
					}
				}
				for (int m = 0; m<b->m_vvInnerBoundary.size(); m++)//for every inner hole
				{
					for (int j = start; j<=end; j++)// for every building in the checking range
					{

						if (mark[j] != 0)//is already in some building's hole
						{
							continue;
						}
						if (j == i)//is itself
						{
							continue;
						}
						for (int k = 0; k < m_vCollection[j]->m_vOutBoundary.size(); k+=3)
						{
							if (m_vCollection[j]->m_vOutBoundary[k]   == b->m_vvInnerBoundary[m][0] &&
								m_vCollection[j]->m_vOutBoundary[k+1] == b->m_vvInnerBoundary[m][1])
							{
								mark[j] = i;
								break;
							}
						}
					}				
				}

			}
		}
	}//end of checking every innerBoundary finding all the one to fill them
	int * isInner = new int[m_vCollection.size()];
	for (int i = 0; i<m_vCollection.size();i++)
	{
		if (mark[i] != 0)
		{
			isInner[i] = 1;
		}
		else
		{
			isInner[i] = 0;
		}
	}
	{
		for (int i = sStart; i<m_vCollection.size(); i++)
		{
			if (mark[i] != 0) // is already in some building's hole
			{
				continue;
			}
			building * b = m_vCollection[i];
			int start = sStart;
			for (int j = i; j>=sStart; j--)
			{
				if (b->m_vOutBoundary[0] - m_vCollection[j]->m_vOutBoundary[0] > 0.004)
				{
					start = j;
					break;
				}
			}
			int end = m_vCollection.size()-1;
			for (int j = i; j<m_vCollection.size(); j++)
			{
				if (m_vCollection[j]->m_vOutBoundary[0] - b->m_vOutBoundary[0] > 0.004)
				{
					end = j;
					break;
				}
			}
			for (int m = start; m<=end; m++)//m is the building to be tested
			{

				if (m == i || isInner[m] != 0 )//is it self or is out of market
				{
					continue;
				}
				int k = 0;
				for (k = 0; k<m_vCollection[m]->m_vOutBoundary.size(); k+=3)
				{
					int l;
					for ( l = 0; l < b->m_vOutBoundary.size(); l+=3)
					{
						if (m_vCollection[m]->m_vOutBoundary[k] == b->m_vOutBoundary[l] &&
							m_vCollection[m]->m_vOutBoundary[k+1] == b->m_vOutBoundary[l+1])
						{						
							if (mark[m] == 0)
							{
								mark[m] = -1;
								mark[i] = m;
							}
							else if (mark[m] == -1)
							{
								mark[i] = m;
							}
							else{
								int t = m;
								while (mark[t] != -1)
								{
									t = mark[t];
								}
								mark[i] = t;
							}
							break;//find a building attached to m_vCollection[i]
						}
					}//end of testing every outerboundary point in m_vCollection[i]
					if (l < b->m_vOutBoundary.size())
					{
						break;
					}
				}//end of testing every outerboundary point in m_vCollection[m] 
				if (k<m_vCollection[m]->m_vOutBoundary.size())
				{
					break;
				}
			}//end of testing every building in range surround m_vCollection[i]
		}//end of testing every buildling start from sStart
	}
	vector<int> *temp  = new vector<int>[m_vCollection.size()];	
	vector<int> *temp2 = new vector<int>[m_vCollection.size()];	

	{
		for (int i = 0; i< m_vCollection.size(); i++)
		{
			temp[i].push_back(i);
		}
		for (int i = sStart; i< m_vCollection.size(); i++)
		{
			if (mark[i] == 0 || mark[i] == -1)
			{
				continue;
			}
			int t = i;
			while (mark[t] != -1 && mark[t] != 0)
			{
				t = mark[t];
			}
			temp[m_vCollection[t]->index].push_back(m_vCollection[i]->index);
		}
		for (int i = sStart; i<m_vCollection.size(); i++)
		{
			sort(temp[i].begin(),temp[i].end());
		}
		for (int i = 0; i< m_vCollection.size(); i++)
		{
			if (temp[i].size() <= 1)
				continue;
			int index = temp[i][0];
			for (int j = 0; j< temp[i].size(); j++)
			{
				temp2[index].push_back(temp[i][j]);
			}
		}
		for (int i = 0; i< m_vCollection.size(); i++)
		{
			if (temp2[i].size()>=1)
			{
				output<<i<<"@";
				for (int j = 1; j<temp2[i].size(); j++)
				{
					output<<temp2[i][j]<<" ";
				}
				output<<endl;

			}
		}
		output.close();
	}
	delete [] temp;
	delete [] mark;
	delete [] temp2;
	delete [] isInner;
}