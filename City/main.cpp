#include "main.h"
#include "City.h"

using namespace std;

//const double SCENE_BACKGROUND_COLOR[] = {0.45, 0.75, 0.9} ;
//const double SCENE_AMBIENT_COLOR[] = {0.7,0.7,0.7};
//
//const double MAP_MAT_AMBIENT_DIFFUSE_COLOR[] = {0.9,0.9,0.9};
//const double MAP_MAT_AMBIENT_COEF = 1.0;
//const double MAP_MAT_DIFFUSE_COEF = 1.0;
//const double BUILDING_MAT_AMBIENT_DIFFUSE_COLOR[] = {0.9,0.9,0.9} ;
//const double BUILDING_MAT_SPECULAR_COLOR[] = {0.0,0.0,0.0} ;
//const double BUILDING_MAT_AMBIENT_COEF = 0.4 ;
//const double BUILDING_MAT_DIFFUSE_COEF = 0.9 ;
//const bool BUILDING_BACKFACE_CULLING = false ;

int main(int argc, char** argv){

	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	City *city = new City();
	city->AddActor(ren);

	cout<<"end of addactor"<<endl;
	ren->SetBackground(0.1, 0.2, 0.4);
	renWin->SetSize(1920, 1080);

	//ren->GetActiveCamera()->SetFocalPoint(0,0,0);
	//ren->GetActiveCamera()->SetPosition(0,50,0);
	//ren->GetActiveCamera()->SetViewUp(0,0,1);
	//ren->GetActiveCamera()->ParallelProjectionOn();
	//ren->ResetCamera();
	//	ren->GetActiveCamera()->Pitch(90);
	//ren->GetActiveCamera()->SetParallelScale(1.5);


	// This starts the event loop and invokes an initial render.
	vtkInteractorStyleTrackballCamera* newin = vtkInteractorStyleTrackballCamera::New();

	iren->SetInteractorStyle(newin);
	iren->Initialize();

	iren->Start();

	return EXIT_SUCCESS;



}