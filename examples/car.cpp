/*
	car.cpp
	Miguel Leitao, 2012

	Vehicle simulation example using btosg abstraction layer.
	Can use a Z pointing up (default) or Y pointing (-D_UP_=0,1,0) up world coordinates reference.
	Vehicle can be interactively controlled from the keyboard:
		up: positive acceleration
		down: negative acceleration.
		right: steering right
		left: steering left
		b: brake
		ESC: quit

*/


#include <osgViewer/Viewer>
#include <osg/Material>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>


#include <osg/Material>
#include <osg/Texture2D>

#define _DEBUG_ (0)

#include "btosgVehicle.h"

int ResetFlag=0;
double frame_time = 0.;

// Create World
btosgWorld myWorld;

btosgBox *myBox;

#include <stack>
std::stack<std::string> mystack;
std::stack<btosgVehicle*> s_V;
btosgVehicle *myVehicle, *myVehicl, *p_V;

// class to handle events
class EventHandler : public osgGA::GUIEventHandler
{
public:
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;
        switch(ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::KEYDOWN):
            switch ( ea.getKey() ) {
            case osgGA::GUIEventAdapter::KEY_Down:
                myVehicle->vehicle->applyEngineForce(-1500, 2);
                myVehicle->vehicle->applyEngineForce(-1500, 3);
                return false;
            case osgGA::GUIEventAdapter::KEY_Up:
                myVehicle->vehicle->applyEngineForce(1500, 2);
                myVehicle->vehicle->applyEngineForce(1500, 3);
                return false;
            case osgGA::GUIEventAdapter::KEY_Left:
                myVehicle->vehicle->setSteeringValue(btScalar(0.4), 0);
                myVehicle->vehicle->setSteeringValue(btScalar(0.4), 1);
                return false;
            case osgGA::GUIEventAdapter::KEY_Right:
                myVehicle->vehicle->setSteeringValue(btScalar(-0.4), 0);
                myVehicle->vehicle->setSteeringValue(btScalar(-0.4), 1);
                return false;
            case 'b':
            case '0':
            case osgGA::GUIEventAdapter::KEY_Control_R:
                myVehicle->vehicle->setBrake(10000, 2);
                myVehicle->vehicle->setBrake(10000, 3);
                return false;
            }
            break;
        case(osgGA::GUIEventAdapter::KEYUP):
            switch ( ea.getKey() ) {
            case osgGA::GUIEventAdapter::KEY_Down:
            case osgGA::GUIEventAdapter::KEY_Up:
                myVehicle->vehicle->applyEngineForce(0, 2);
                myVehicle->vehicle->applyEngineForce(0, 3);
                return false;
            case osgGA::GUIEventAdapter::KEY_Left:
            case osgGA::GUIEventAdapter::KEY_Right:
                myVehicle->vehicle->setSteeringValue(btScalar(0), 0);
                myVehicle->vehicle->setSteeringValue(btScalar(0), 1);
                return false;
            case '0':
            case 'b':
            case osgGA::GUIEventAdapter::KEY_Control_R:
                myVehicle->vehicle->setBrake(5, 2);
                myVehicle->vehicle->setBrake(5, 3);
                return false;
            case 'S':
                std::cout << "tecla S" << std::endl;
                return false;
            case 'i':
                myVehicle->printInfo();
                break;
            case 'f':
                std::cout << "adding force" << std::endl;
                myBox->body->activate(true);
                myBox->body->applyCentralImpulse(btVector3(100.,0.,0.));
                return false;
            case 'F':
                std::cout << "adding Force" << std::endl;

                myVehicle->vehicle->applyEngineForce(500, 2);
                myVehicle->vehicle->applyEngineForce(500, 3);

                int i;
                for( i=0 ; i<myVehicle->vehicle->getNumWheels() ; i++) {
                    btWheelInfo& iWheel = myVehicle->vehicle->getWheelInfo(i);
                    printf(" wheel %d, radius %f, rotation %f, eforce %f, steer %f\n",
                           i, iWheel.m_wheelsRadius, iWheel.m_rotation, iWheel.m_engineForce,iWheel.m_steering);
                }
                // handled = true;
                return false;
            case 'R':
            case 'r':
                ResetFlag = 1;
                std::cout << "tecla R" << std::endl;
                break;
            }
            break;
        case(osgGA::GUIEventAdapter::MOVE):
            //std::cout << "mouse move " << ea.getX() << " " << ea.getY() << std::endl;
            return false;
        default:
            return false;
        }
        return true;
    }
};


class BlockGreen : public btosgBox {
public:
    BlockGreen(float x, float y, float z) : btosgBox( osg::Vec3(1.,1.,1.), 100. ) {
        setPosition(btVector3(x,y,z));
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0., 0., 0., 1.0));
        mat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.1, 0.5, 0.1, 1.0));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1.0));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 64);
        model->getOrCreateStateSet()->
        setAttributeAndModes(mat, osg::StateAttribute::ON);
    }
    BlockGreen(float x, float z) : BlockGreen(x,3.,z) {};
};

class BlockRed : public btosgBox {
public:
    BlockRed(float x, float y, float z) : btosgBox( osg::Vec3(1.,1.,1.), 5000. ) {
        setPosition(btVector3(x,y,z));
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0., 0., 0., 1.0));
        mat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.6, 0.1, 0.1, 1.0));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1.0));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 64);
        model->getOrCreateStateSet()->
        setAttributeAndModes(mat, osg::StateAttribute::ON);
    }
    BlockRed(float x, float z) : BlockRed(x,3.,z) {};
};

class BlockBlue : public btosgBox {
public:
    BlockBlue(float x, float y, float z) : btosgBox( osg::Vec3(1.,0.25,1.), 10000. ) {
        setPosition(btVector3(x,y,z));
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0., 0., 0., 1.0));
        mat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.1, 0.1, 0.5, 1.0));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1.0));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 64);
        model->getOrCreateStateSet()->
        setAttributeAndModes(mat, osg::StateAttribute::ON);
    }
    BlockBlue(float x, float z) : BlockBlue(x,3.,z) {};
};


// Phos Stack Machine 2020-08-06
std::stack<void*> sm_S;
std::stack<std::string> sm_SS;

void *sm_pop()
{
  void *t = sm_S.top();
  sm_S.pop();
  
  return t;

}

int sm_newcar()
{
    btosgVec3 up(0., 0., 1.);
#ifdef _UP_
    up = btosgVec3(_UP_);
#endif

    s_V.push(new btosgVehicle(&myWorld));
    p_V = s_V.top();
    p_V->setPosition(btosgVec3(up*3.));
    p_V->setName("Vehicle");
    p_V->setMass(800.);
    myWorld.addObject( p_V );
    p_V->printInfo();
    
    return 0;
}


void foo() { std::cout << "foo()"; }
void boo() { std::cout << "boo()"; }
void too() { std::cout << "too()"; }
void goo() { std::cout << "goo()"; }

int map_func(std::string func) {
  std::map<std::string, std::function<void()>> functions;
  functions["foo"] = foo;
  functions["boo"] = boo;
  functions["too"] = too;
  functions["goo"] = goo;
  // functions["svp:"] = sm_svp;
  // functions[ "dJointSetHinge2Param:" ] = sm_dJointSetHinge2Param;
  functions["newcar:"] = sm_newcar;
  functions["p:"] = sm_pop;

  // std::string func;
  // std::cin >> func;
  if (functions.find(func) != functions.end()) {
    functions[func]();
  }
  return 0;
}


template <typename T>
using is_string = std::is_constructible<std::string, T>;

template <typename T> const char* sm_typeof(T&) { return "unknown"; }    // default
template<> const char* sm_typeof(int&) { return "int"; }
template<> const char* sm_typeof(float&) { return "float"; }
template<> const char* sm_typeof(std::string&) { return "string"; }
template<> const char* sm_typeof(float*&) { return "ptr_float"; }
template<> const char* sm_typeof(void*&) { return "ptr_void"; }
template<> const char* sm_typeof(char*&) { return "ptr_char"; }


// template functions sm_lastchar
template <typename T> const char* sm_lastchar(T& t) { 
std::cout << "    DEFAULT sm_lastchar " << t << "  is_string " << is_string<decltype(t)>::value <<std::endl;
return "unknown "; } 

template<> const char* sm_lastchar(int& t) { 
std::cout << "    int sm_lastchar " <<  t <<std::endl;
return "unknown "; } 

/*
template<> const char* sm_lastchar(dReal& t) { 
std::cout << "    dReal sm_lastchar " <<  t <<std::endl;
return "unknown "; } 

template<> const char* sm_lastchar(dJointID& t) { 
std::cout << "    dJointID sm_lastchar " <<  t <<std::endl;
return "unknown "; } 

template<> const char* sm_lastchar(dParamVel2& t) { 
std::cout << "    dParamVel2 sm_lastchar " <<  t <<std::endl;
return "unknown "; } 
*/

template<> const char* sm_lastchar(std::string& t) 
{ 
    std::cout << "    sm_lastchar " << t.back() <<std::endl;
    
    if (t.back()==':') {
        std::cout << "    is colon " << t.back() <<std::endl;

        map_func(t);
    }
    return "string "+t.back(); 
}

template<> const char* sm_lastchar(float*& t) { 
    sm_S.push((void*)t);
    return "ptr_float"; 
}

// const char* sm_lastchar(std::string&) { return s.back(); }
// const char* sm_lastchar(std::string&) { return s.back(); }

template <typename T>
void funcvar(T t) 
{
    // std::cout << t << std::endl ;
    
    std::cout << t << ' ' ;
    std::cout << typeid(t).name() << ' ' ;
    std::cout << std::is_function<decltype(t)>::value << ' ';
    std::cout << sm_typeof(t) <<std::endl  ;
}

template<typename T, typename... Args>
void funcvar(T t, Args... args) // recursive variadic function
{
    std::cout << t << ' ' ;
    std::cout << typeid(t).name() << ' ' ;
    std::cout << std::is_function<decltype(t)>::value << ' ';
    std::cout << sm_typeof(t) <<std::endl  ;

    funcvar(args...) ;
}




// sm_proc recursive variadic 
template <typename T>
void sm_proc(T t) 
{
    // std::cout << t << std::endl ;
    
    std::cout << t << ' ' ;
    std::cout << typeid(t).name() << ' ' ;
    std::cout << std::is_function<decltype(t)>::value << ' ';
    std::cout << "    is_string " << is_string<decltype(t)>::value << ' ';
    std::cout << sm_typeof(t) <<std::endl  ;
    
    if ( (is_string<decltype(t)>::value)==1 ) {
      // std::cout << "    last char " << t.back() <<std::endl  ;
      // std::cout << "    last char "  <<std::endl  ;
      std::string s_t(t);
      std::cout << "    s_t " << s_t << "    last char " << sm_lastchar(s_t) <<std::endl  ;
    }
    else sm_lastchar(t); // sm_S.push(t);
}



template<typename T, typename... Args>
void sm_proc(T t, Args... args) // recursive variadic function
{
    std::cout << t << ' ' ;
    std::cout << typeid(t).name() << ' ' ;
    std::cout << std::is_function<decltype(t)>::value << ' ';
    std::cout << "    is_string " << is_string<decltype(t)>::value << ' ';
    std::cout << sm_typeof(t) <<std::endl  ;

    // if (1) {
    if ( (is_string<decltype(t)>::value)==1 ) {
      // std::cout << "    last char " << t.back() <<std::endl  ;
      // std::cout << "    last char "  <<std::endl  ;
      std::cout << "    last char " << sm_lastchar(t) <<std::endl  ;
    }
    else sm_lastchar(t); // sm_S.push(t);

    sm_proc(args...) ;
}


int main()
{
    btosgVec3 up(0., 0., 1.);
#ifdef _UP_
    up = btosgVec3(_UP_);
#endif
    btosgVec3 gravity = up*-9.8;
    myWorld.dynamic->setGravity(gravity);

    // Car
    myVehicle = new btosgVehicle(&myWorld);
    myVehicle->setPosition(btosgVec3(up*3.));
    myVehicle->setName("Vehicle");
    myVehicle->setMass(800.);
    myWorld.addObject( myVehicle );
    myVehicle->printInfo();
    
    // Phoscript 2020-08-13
    
    if (1) {    
    // Car
        myVehicl = new btosgVehicle(&myWorld);
        myVehicl->setPosition(btosgVec3(up*3.));
        myVehicl->setName("Vehicle");
        myVehicl->setMass(800.);
        myWorld.addObject( myVehicl );
        myVehicl->printInfo();
        

        s_V.push(new btosgVehicle(&myWorld));
        p_V = s_V.top();
        p_V->setPosition(btosgVec3(up*3.));
        p_V->setName("Vehicle");
        p_V->setMass(800.);
        myWorld.addObject( p_V );
        p_V->printInfo();
        
        sm_newcar();
        
        sm_proc( "newcar:" );
    }

    {
        BlockGreen *myBlock;
        myBlock = new BlockGreen(4.,-4.);
        myWorld.addObject(myBlock);
        myBlock = new BlockGreen(6.,5.);
        myWorld.addObject(myBlock);
        myBlock = new BlockGreen(0.,0.);
        myWorld.addObject(myBlock);
        myBlock = new BlockGreen(9.,5.);
        myWorld.addObject(myBlock);
        myBlock = new BlockGreen(10.,1.);
        myWorld.addObject(myBlock);
        myBlock = new BlockGreen(-11.,6.);
        myWorld.addObject(myBlock);
    }
    {
        BlockRed *myBlock;
        myBlock = new BlockRed(4.,4.);
        myWorld.addObject(myBlock);
        myBlock = new BlockRed(7.,5.);
        myWorld.addObject(myBlock);
        myBlock = new BlockRed(-8.,5.);
        myWorld.addObject(myBlock);
        myBlock = new BlockRed(9.,-5.);
        myWorld.addObject(myBlock);
        myBlock = new BlockRed(10.,-6.);
        myWorld.addObject(myBlock);
        myBlock = new BlockRed(-12.,6.);
        myWorld.addObject(myBlock);
    }

    {
        BlockBlue *myBlock;
        myBlock = new BlockBlue(4.,-4.);
        myWorld.addObject(myBlock);
        myBlock = new BlockBlue(7.5,6.);
        myWorld.addObject(myBlock);
        myBlock = new BlockBlue(-8.,-5.);
        myWorld.addObject(myBlock);
        myBlock = new BlockBlue(9.,5.);
        myWorld.addObject(myBlock);
        myBlock = new BlockBlue(11.,-7.);
        myWorld.addObject(myBlock);
        myBlock = new BlockBlue(-13.,7.);
        myWorld.addObject(myBlock);
    }

    // Wheels
    osg::ref_ptr<osg::Material> matCylinder = new osg::Material;
    matCylinder->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.,  0.,  1.0));
    matCylinder->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.6, 0.4, 0.1, 1.0));
    matCylinder->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.,  0.,  0.,  1.0));
    matCylinder->setShininess(osg::Material::FRONT_AND_BACK, 64);

    // Plane
    btosgPlane *myRamp = new btosgPlane(osg::Vec3(50.,50.,50.) - up*50.);
    myRamp->setPosition(0.,0.,0.);
    myWorld.addObject( myRamp );
    myRamp->setName("Ramp");
    myRamp->body->setFriction(100.);
    osg::ref_ptr<osg::Material> matRamp = new osg::Material;
    matRamp->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0., 0., 0., 1.0));
    matRamp->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.7, 0.8, 0.0, 1.0));
    matRamp->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1.0));
    matRamp->setShininess(osg::Material::FRONT_AND_BACK, 64);
    myRamp->setMaterial(matRamp);


    // Creating the viewer
    osgViewer::Viewer viewer ;

    // Setup camera
    osg::Matrix matrix;
    matrix.makeLookAt( osg::Vec3(0.,8.,5.), osg::Vec3(0.,0.,1.), up );
    viewer.getCamera()->setViewMatrix(matrix);

    // add the Event handler
    viewer.addEventHandler(new EventHandler());

    // Light
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
    ls->getLight()->setPosition(osg::Vec4(2.5,-10+30*up[1],-10+30.*up[2],1.));
    ls->getLight()->setAmbient(osg::Vec4(0.1, 0.1, 0.1, 1.0));
    ls->getLight()->setDiffuse(osg::Vec4(1.0, 1.0, 1.0, 1.0));
    ls->getLight()->setSpecular(osg::Vec4(0.2, 0.2, 0.2, 1.0));
    myWorld.scene->addChild(ls.get());

    viewer.setSceneData( myWorld.scene );

    osg::ref_ptr<osgGA::TrackballManipulator> manipulator = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator( manipulator );

    // Set the desired home coordinates for the manipulator
    osg::Vec3d eye(osg::Vec3(0., -5., -5.)+up*20.);
    osg::Vec3d center(0., 0., 0.);

    // Make sure that OSG is not overriding our home position
    manipulator->setAutoComputeHomePosition(false);
    // Set the desired home position of the Trackball Manipulator
    manipulator->setHomePosition(eye, center, up);
    // Force the camera to move to the home position
    manipulator->home(0.0);

    // record the timer tick at the start of rendering.
    osg::Timer myTimer;
    double timenow = myTimer.time_s();
    double last_time = timenow;
    frame_time = 0.;

    while( !viewer.done() )
    {
        myWorld.stepSimulation(frame_time,10);
        viewer.frame();
        timenow = myTimer.time_s();
        frame_time = timenow - last_time;
        last_time = timenow;

        if (ResetFlag>0) {
            myWorld.reset();
            ResetFlag = 0;
        }
    }
}

