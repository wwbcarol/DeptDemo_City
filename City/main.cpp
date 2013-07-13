#include "main.h"
#include "City.h"

using namespace std;

int main(int argc, char** argv){

	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	City *city = new City();
	city->AddActor(ren);

	ren->SetBackground(0.1, 0.2, 0.4);
	renWin->SetSize(1920, 1080);

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



}