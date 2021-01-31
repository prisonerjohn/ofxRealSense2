#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
	
    int w = 640;
    int h = 480;

    // setup the GL context
#ifdef TARGET_LINUX_ARM
#ifdef TARGET_OPENGLES
    ofGLESWindowSettings settings;
    settings.glesVersion = 2;
    settings.setSize(w, h);
    ofCreateWindow(settings);
#else
    ofGLFWWindowSettings settings;
    settings.numSamples = 0;
    settings.setSize(w, h);
    ofCreateWindow(settings);                       
#endif
#else
    ofSetupOpenGL(w, h, OF_WINDOW);
#endif

    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());

}
