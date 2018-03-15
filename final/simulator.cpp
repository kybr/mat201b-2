#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "allocore/math/al_Ray.hpp"
#include <cmath> 
#include "planet.cpp"
#include "common.cpp"

using namespace al;
using namespace std;
// Mesh sphere;  

//helper to find file
string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(whereToLook);
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}

struct MyApp : App {

  State* state = new State;
  cuttlebone::Maker<State> maker;

  Material material;
  Light light;

  vector<Planet> planets;
  Planet special;
  Planet myPlanet;
  bool simulate = true;

  Vec3f savePos;

  //background related
  Mesh bgMesh;
  Texture bgTexture;

  MyApp(): maker("127.0.0.1"){
    addSphereWithTexcoords(bgMesh);
 // load image into texture print out error and exit if failure
    Image image;
    if (image.load(fullPathOrDie("cell.jpg"))) {
      cout << "Read image from "<< endl;
    } else {
      cout << "Failed to read image from " <<  "!!!" << endl;
      exit(-1);
    }

    bgTexture.allocate(image.array());
   
    //initial pos/light/lens
    light.pos(0, 0, 0);         
    nav().pos(0, 0, 0);        
    lens().far(400);     

    planets.resize(particleCount);
    myPlanet.setMe();

    background(Color(0.07));
    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {
    //nav().pos(0,0,0);

    //pressed s to pause/resume the game
    if (!simulate)
      return;

    for(int i = 0; i<planets.size(); i++){
      for (int j = i + 1;j<planets.size(); j++){
          if(planets[i].ifCollide(planets[j])){
            if(planets[i].volume>planets[j].volume){
              planets[i].absorb(planets[j]);
            }else{
              planets[j].absorb(planets[i]);
            }
          }
      }
    }

    
    for(int i = 0; i<planets.size(); i++){
      if(myPlanet.ifCollide(planets[i])){
         if(myPlanet.volume>planets[i].volume){
              myPlanet.absorb(planets[i]);
            }else{
              //simulate = false;
            }
      }
    }

    checkIfExist(planets, myPlanet);
    //update function for each planet
    for (auto& p : planets) p.update(myPlanet);
    myPlanet.update(myPlanet);

    nav().faceToward(myPlanet.position, Vec3d(0,1,0),0.05);

    //cuttlebone settings
    
    for (unsigned i = 0; i < planets.size(); i++){
       state->position[i] = planets[i].position;
       state->rad[i] = planets[i].rad;
    }
    state-> myPosition = myPlanet.position;
    state-> myRad = myPlanet.rad;
    maker.set(*state);
  }

  virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){
    Rayd r = getPickRay(w, m.x(), m.y());
    // cout<<"r: "<<r.direction()<<endl;
    float t = r.intersectSphere(Vec3d(0,0,0), placeholderSize);
    if(t > 0.f){
    }
    // r(t)
    cout<<"r: "<<r(placeholderSize)<<endl;
    Vec3f newVelocity = cross(Vec3f(r(placeholderSize)),myPlanet.position).normalize();
    myPlanet.velocity = newVelocity;
    myPlanet.speed = 0.015;
    
    savePos = r(placeholderSize);
   // myPlanet.speed = 0.01;
  }

  void onDraw(Graphics& g) {
    g.lighting(false);  // turn off lighting
    g.depthMask(false);  // disable depth buffer, so that background will be drawn over

    g.pushMatrix();
    g.translate(nav().pos());
    g.rotate(180, 0, 0, 1);
    bgTexture.bind();
    g.color(1, 1, 1);
    g.draw(bgMesh);
    bgTexture.unbind();
    g.popMatrix();

    g.depthMask(true);  // turn depth mask back on

    material();
    light();
    //bgTexture.quad(g);
    g.scale(scaleFactor);
    myPlanet.draw(g);
    for (auto& b : planets) b.draw(g);
    Mesh& m = g.mesh();
    addCone(m);
    g.translate(savePos);
    g.draw(m);

  }


 void onKeyDown(const Keyboard& k){
		switch(k.key()){
      case 's':
        cout<< "pressed s"<<endl;
        simulate = !simulate;
        break;
		}
	}

  //check if any planet has volume less than 0. If yes, delete them
  void checkIfExist(vector<Planet>& planets, Planet& myPlanet){
    vector<int> deletingPlanet;
    for(int i = 0; i<planets.size(); i++){
        Planet planet = planets[i];
        if(planet.volume < 0){
          deletingPlanet.push_back(i);
        }
    }
    for(auto index: deletingPlanet){
      planets.erase(planets.begin()+index);
    }
    if (myPlanet.volume <0){
      simulate = false;
    }
  }
 };

int main() {   
  MyApp app;
  app.maker.start();
  app.start(); }
