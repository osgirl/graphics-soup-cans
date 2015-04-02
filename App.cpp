/*
    Comp 394-01 S15 
    Soup Cans Activity
    Marcio Porto and Charles Park
*/

#include "App.h"
#include <iomanip>
using namespace std;

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    (void)argc; (void)argv;
    GApp::Settings settings(argc, argv);
    
    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.width       = 1280;
    settings.window.height      = 720;
    settings.window.resizable = true;
    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {
    renderDevice->setColorClearValue(Color3(0.3,0.3,0.3));
    renderDevice->setSwapBuffersAutomatically(true);
}


void App::onInit() {
    // Turn on the developer HUD
    createDeveloperHUD();
    debugWindow->setVisible(false);
    developerWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    showRenderingStats = false;
    
    activeCamera()->setPosition(Vector3(0,0,3));
    activeCamera()->lookAt(Vector3(0,0,0), Vector3(0,1,0));
    activeCamera()->setFarPlaneZ(-400);
    
    rotation = Matrix3::identity();
    
    light = Light::directional("downlight", Vector3(0, 1, 0.7), Color3::white());
    
    // Setup geometry
    Array<Vector3> cpuVerts;
    Array<Vector3> cpuNorms;
    Array<Vector2> cpuTexCoords;
    Array<int> cpuIndices;
    
    
    int numslices = 60;
    float r = 0.75;
    
    // Draw top
    cpuVerts.append(Vector3(0,1,0));
    cpuNorms.append(Vector3(0,1,0));
    cpuTexCoords.append(Vector2(0.43, 0.5));//0.5, 0.5));
    for (int slice=0; slice<numslices; slice++) {
        float theta = (float)slice/(float)numslices * 2.0 * pi();
        Vector3 p(r*sin(theta), 1.0, r*cos(theta));
        cpuVerts.append(p);
        cpuNorms.append(Vector3(0,1,0));
        Vector2 st(0,1);//sin(theta)/2.0+0.5, cos(theta)/2.0+0.5);
        cpuTexCoords.append(st);
        
        if (slice > 0) {
            // add a triangle
            cpuIndices.append(0);
            cpuIndices.append(slice);
            cpuIndices.append(slice+1);
        }
    }
    // Add the last triangle connecting to the first vertex
    cpuIndices.append(0);
    cpuIndices.append(cpuVerts.size()-1);
    cpuIndices.append(1);
    
    
    int first = cpuVerts.size();
    //Draw bottom
    cpuVerts.append(Vector3(0,-1,0));
    cpuNorms.append(Vector3(0,-1,0));
    cpuTexCoords.append(Vector2(0.43, 0.5));
    for (int slice=0; slice<numslices; slice++) {
        float theta = (float)slice/(float)numslices * 2.0 * pi();
        Vector3 p(r*cos(theta), -1.0, r*sin(theta));
        cpuVerts.append(p);
        cpuNorms.append(Vector3(0,-1,0));
        Vector2 st(0, 1);//sin(theta)/2.0+0.5, cos(theta)/2.0+0.5);
        cpuTexCoords.append(st);
        
        if (slice > 0) {
            // add a triangle
            cpuIndices.append(first);
            cpuIndices.append(first+slice);
            cpuIndices.append(first+slice+1);
        }
    }
    // Add the last triangle connecting to the first vertex
    cpuIndices.append(first);
    cpuIndices.append(cpuVerts.size()-1);
    cpuIndices.append(first+1);
    
    first = cpuVerts.size();
    // Make connections between top and bottom
    for (int slice=0; slice<=numslices; slice++) {
        float theta = (float)slice/(float)numslices * 2.0 * pi() + pi(); // Add pi to rotate the texture around. Could also have just modified the texture coordinates
        Vector3 normal(r*sin(theta), 0.0, r*cos(theta));
        normal = normal.unit();
        Vector3 pTop(r*sin(theta), 1.0, r*cos(theta));
        cpuVerts.append(pTop);
        cpuNorms.append(normal);
        float textureCoord = (float)slice/(float)numslices;
        Vector2 st(textureCoord, 0);
        cpuTexCoords.append(st);
        
        Vector3 pBottom(r*sin(theta), -1.0, r*cos(theta));
        cpuVerts.append(pBottom);
        cpuNorms.append(normal);
        st = Vector2(textureCoord, 1.0);
        cpuTexCoords.append(st);
        
        if (slice > 0) {
            // add a triangle
            cpuIndices.append(first+(slice*2)+1);
            cpuIndices.append(first+(slice*2));
            cpuIndices.append(first+(slice*2)-2);
            
            cpuIndices.append(first+(slice*2)+1);
            cpuIndices.append(first+(slice*2)-2);
            cpuIndices.append(first+(slice*2)-1);
            
        }
    }

    
    
    // For each vertex store: (1) Vector3 for x,y,z position, (2) Vector3 for x,y,z Normal, (3) Vector2 for s,t Texture Coordinates
    // Plus, for each triangle store: 3 integer indices into the vextex array.
    vbuffer = VertexBuffer::create(cpuVerts.size()*(sizeof(Vector3)+sizeof(Vector3)+sizeof(Vector2)) + sizeof(int)*cpuIndices.size());
    
    gpuVerts = AttributeArray(cpuVerts, vbuffer);
    gpuNorms = AttributeArray(cpuNorms, vbuffer);
    gpuTexCoords = AttributeArray(cpuTexCoords, vbuffer);
    gpuIndices = IndexStream(cpuIndices, vbuffer);
    
    tex = Texture::fromFile("/Users/mporto/Documents/Graphics/Texturing-Starter/campbells.jpg");
    
    G3D::String vertexShaderPath = "texture-shader.vrt";
    G3D::String fragmentShaderPath = "texture-shader.pix";
    
    debugAssert(FileSystem::exists(vertexShaderPath));
    debugAssert(FileSystem::exists(fragmentShaderPath));
    shader = Shader::fromFiles(vertexShaderPath, fragmentShaderPath);
}

void App::onUserInput(UserInput *ui) {
    // Dolly the camera closer or farther away from the earth
    if (ui->keyDown(GKey::UP)) {
        Vector3 newCamPos = activeCamera()->frame().translation + Vector3(0,0,-0.01);
        if (newCamPos[2] > 1.2) {
            activeCamera()->setPosition(newCamPos);
        }
    }
    if (ui->keyDown(GKey::DOWN)) {
        Vector3 newCamPos = activeCamera()->frame().translation + Vector3(0,0,0.01);
        activeCamera()->setPosition(newCamPos);
    }
    
    // Rotate the earth when the user clicks and drags the mouse
    if (ui->keyDown(GKey::LEFT_MOUSE)) {
        Vector2 dxy = ui->mouseDXY();
        if (dxy.length() > 0) {
            Vector3 axis(dxy.y, dxy.x, 0);
            rotation = Matrix3::fromAxisAngle(axis, dxy.length()/500.0) * rotation;
        }
    }

}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);
    
}

void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
    rd->clear();
    rd->setCullFace(CullFace::BACK);
    
    rd->pushState();
    Args args;
    args.setUniform("diffuseColor", Color3::white());
    
    args.setUniform("wsLight", light->position().xyz().direction());
    args.setUniform("lightColor", light->color);
    
    args.setAttributeArray("vertex", gpuVerts);
    args.setAttributeArray("normal", gpuNorms);
    
    args.setIndexStream(gpuIndices);
    args.setPrimitiveType(PrimitiveType::TRIANGLES);
    
    args.setAttributeArray("texCoord0", gpuTexCoords);
    
    Sampler s(WrapMode::TILE, InterpolateMode::NEAREST_NO_MIPMAP);
    args.setUniform("textureSampler", tex,s);// Sampler::video());
    
    //rd->setRenderMode(RenderDevice::RENDER_WIREFRAME);
    
    rd->setObjectToWorldMatrix(CoordinateFrame(rotation, Vector3(0,0,0)));
    rd->apply(shader, args);
    
    rd->popState();
    
    
    // Call to make the GApp show the output of debugDraw
    drawDebugShapes();
    //Draw::axes(CoordinateFrame(), rd);
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    Surface2D::sortAndRender(rd, posed2D);
}


