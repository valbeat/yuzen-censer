#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofSetBackgroundAuto(false); //フレーム更新時に塗り潰しを無効化
    ofBackground(0, 0, 0); //背景色を黒に設定
    
    sender.setup(HOST, PORT);
    
    ofToggleFullscreen();
    
    camWidth = ofGetWidth();
    camHeight = ofGetHeight();
    
    #ifdef _USE_LIVE_VIDEO
        camera.listDevices();
        camera.setVerbose(true);
        camera.setDeviceID(0);
        camera.initGrabber(camWidth,camHeight);
    #else
        //video.loadMovie();
        //video.play();
    #endif
    


    
    //背景の学習を設定
    background.setLearningTime(900);
    background.setThresholdValue(20);

    
    //ボタンの動作設定
    resetBackgroundButton.addListener(this, &ofApp::resetBackgroundPressed);
    
    //GUIの設定
    gui.setup();
    gui.add(minRad.setup("Min Area", 10, 0, 100));
    gui.add(maxRad.setup("Max Area", 100, 0, 1000));
    gui.add(minArea.setup("Min Area", 0, 0, 1));
    gui.add(maxArea.setup("Max Area", 1, 0, 1));
    gui.add(medianScale.setup("median Blur Size", 1, 0, 50));
    gui.add(bgThresh.setup("background thresh", 50, 0, 100));
    gui.add(contourThresh.setup("contour finder thresh", 500, 0, 1000));
    gui.add(resetBackgroundButton.setup("reset background"));
    gui.add(learnBgFlag.setup("gui",true));
    gui.add(diffFlag.setup("diff image",true));
    gui.add(contourFlag.setup("contour image",true));
    gui.add(bgFlag.setup("background image",false));
    gui.add(cameraFlag.setup("camera image",true));
    //    gui.add(flowScale.setup("flowScale", 0.05, 0.0, 1.0));
    //    gui.add(pyrScale.setup("pyrScale", 0.5, 0, 1));
    //    gui.add(levels.setup("levels", 4, 1, 8));
    //    gui.add(winSize.setup("winsize", 8, 4, 64));
    //    gui.add(iterations.setup("iterations", 2, 1, 8));
    //    gui.add(polyN.setup("polyN", 7, 5, 10));
    //    gui.add(polySigma.setup("polySigma", 1.5, 1.1, 2));
    //    gui.add(OPTFLOW_FARNEBACK_GAUSSIAN.setup("OPTFLOW_FARNEBACK_GAUSSIAN",false));
    //    gui.add(useFarneback.setup("use Farnback",true));
    //    gui.add(qualityLevel.setup("quality level", 0.01, 0.001, 0.02));
    //    gui.add(maxLevel.setup("max level", 3,0,8));
    //    gui.add(minDistance.setup("minDistance", 4, 1, 16));
    gui.add(fullScreenToggle.setup("full screen",true));
    gui.add(guiFlag.setup("gui",true));

}

//--------------------------------------------------------------
void ofApp::update(){
    camera.update();
    if(camera.isFrameNew()) {
        ofxCv::medianBlur(camera, camera, medianScale); //ノイズがあるので平滑化
        if (learnBgFlag) {
            // 背景差分を取る
            background.setThresholdValue(bgThresh);
            background.update(camera, diffImg);
            diffImg.update();
        } else {
            // フレーム差分を取る
            
        }
        //輪郭の設定
        contourFinder.setThreshold(contourThresh);
        contourFinder.setMinAreaRadius(minRad);
        contourFinder.setMaxAreaRadius(maxRad);
        contourFinder.setMinArea(minArea * minArea * camWidth * camHeight);
        contourFinder.setMaxArea(maxArea * maxArea * camWidth * camHeight);
        contourFinder.findContours(diffImg);
        sendContourPosition();
        
        
    }
    
}
//--------------------------------------------------------------
void ofApp::draw(){
    ofNoFill();
    
    if (cameraFlag) {
        camera.draw(0, 0, camWidth, camHeight);
    }
    if (bgFlag) {
        bgImg.draw(0, 0, camWidth, camHeight);
    }
    if (diffFlag) {
        ofSetColor(255);
        diffImg.draw(0, 0, camWidth, camHeight);
    }
    if (contourFlag) {
        ofSetColor(0, 255, 255);
        contourFinder.draw();
    }
    if (guiFlag) {
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case 'f':
            ofToggleFullscreen();
            break;
        case ' ':
            background.reset();
            break;
        case 'g':
            (guiFlag == false) ? guiFlag = true : guiFlag = false;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    camWidth = ofGetWidth();
    camHeight = ofGetHeight();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
void ofApp::resetBackgroundPressed() {
    background.reset();
}
//--------------------------------------------------------------
void ofApp::toggleFullScreenPressed() {
    ofToggleFullscreen();
}
//--------------------------------------------------------------
void ofApp::dumpOSC(ofxOscMessage m) {
    String msg_string;
    msg_string = m.getAddress();
    for (int i=0; i<m.getNumArgs(); i++ ) {
        msg_string += " ";
        if(m.getArgType(i) == OFXOSC_TYPE_INT32)
            msg_string += ofToString( m.getArgAsInt32(i));
            else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT)
                msg_string += ofToString( m.getArgAsFloat(i));
                else if(m.getArgType(i) == OFXOSC_TYPE_STRING)
                    msg_string += m.getArgAsString(i);
                    }
    cout << msg_string << endl;
}
//--------------------------------------------------------------
void ofApp::sendContourPosition() {
    RectTracker tracker = contourFinder.getTracker();
    float x,y,z;
    for (int i = 0; i < contourFinder.size(); i++) {
        // TODO:座標を重心に設定する
        x = contourFinder.getBoundingRect(i).x;
        y = contourFinder.getBoundingRect(i).y;
        // z方向は面積から取得
        z = contourFinder.getBoundingRect(i).area();
        z = z / 1000;
        ofxOscMessage m;
        m.setAddress( "/user/position" );
        m.addIntArg(i);
        m.addIntArg(x);
        m.addIntArg(y);
        m.addIntArg(z);
        m.addIntArg(oscCount);
        sender.sendMessage(m);
        dumpOSC(m);
        oscCount++;
    }
}
//--------------------------------------------------------------
void ofApp::sendFlowVector(){
    // TODO:flowベクターを送る
    
}