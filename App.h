/**
	Comp 394-01 S15 
	Soup Cans Activity
	Marcio Porto and Charles Park
**/

#ifndef App_h
#define App_h

#include <G3D/G3DAll.h>

class App : public GApp {
public:

	App(const GApp::Settings& settings = GApp::Settings());

	virtual void onInit();

	virtual void onUserInput(UserInput *uinput);

	virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt);

	virtual void onGraphics3D(RenderDevice* rd, Array< shared_ptr<Surface> >& surface);
	virtual void onGraphics2D(RenderDevice* rd, Array< shared_ptr<Surface2D> >& surface2D);

private:
	shared_ptr<Light> light;
    shared_ptr<Shader> shader;
    shared_ptr<Texture> tex;
    shared_ptr<Texture> campbells;

	// VertexBuffer for storing positions, texture coordinates, and normals
	shared_ptr<VertexBuffer> vbuffer;

	// Per-vertex 3D position data
	AttributeArray gpuVerts;

	// Per-vertex 3D normals data
	AttributeArray gpuNorms;
    
    // Per-vertex s,t-texture coordinate data
    AttributeArray gpuTexCoords;

	// Index data for sending vertices
	IndexStream  gpuIndices;

	Matrix3 rotation;

};

#endif