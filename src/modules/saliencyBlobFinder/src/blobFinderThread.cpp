// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include <iCub/blobFinderThread.h>
#include <cstring>
#include <iostream>

using namespace std;
using namespace yarp::os;

const int defaultSize = 240;

//#define _inputImg (*(ptr_inputImg))

#define _inputImgRed (*(ptr_inputImgRed))
#define _inputImgGreen (*(ptr_inputImgGreen))
#define _inputImgBlue (*(ptr_inputImgBlue))

#define _inputImgRG (*(ptr_inputImgRG))
#define _inputImgGR (*(ptr_inputImgGR))
#define _inputImgBY (*(ptr_inputImgBY))

#define _tagged (*(ptr_tagged))

#define SPIKE_COUNTS 10

const int THREAD_RATE=500;

bool logpolarToCart(yarp::sig::ImageOf<yarp::sig::PixelRgb>& cart,const yarp::sig::ImageOf<yarp::sig::PixelRgb>& lp,lp2CartPixel *l2cTable) {
    // adjust padding.
    if (lp.getPadding() != 0) {
        int i;
        const int byte = lp.width() * sizeof(PixelRgb);
        unsigned char *d = lp.getRawImage();
        for (i = 1; i < lp.height(); i ++) {
            unsigned char *s = (unsigned char *)lp.getRow(i);
            memmove(d, s, byte);
            d += byte;
        }
    }

    // LATER: assert whether lp & cart are effectively of the correct size.
    RCgetCartImg (cart.getRawImage(), lp.getRawImage(), l2cTable, cart.width() * cart.height());

    // adjust padding.
    if (cart.getPadding() != 0) {
        const int byte = cart.width() * sizeof(PixelRgb);
        int i;
        for (i = cart.height()-1; i >= 1; i--) {
            unsigned char *d = cart.getRow(i);
            unsigned char *s = cart.getRawImage() + i*byte;
            memmove(d, s, byte);
        }
    }
    return true;
}

blobFinderThread::blobFinderThread():RateThread(THREAD_RATE)
{
    ippSetNumThreads(1);
    reinit_flag=false;
    interrupted_flag=false;
    ct=0;
    filterSpikes_flag=false;
    count=0;
    //inputImage_flag=false;
    freetorun=false;

    l2cTable=0;

    /*meanColour_flag=true;
    contrastLP_flag=false;
    blobCataloged_flag=true;
    foveaBlob_flag=false;
    colorVQ_flag=false;
    blobList_flag=false;
    maxSaliencyBlob_flag=false;
    tagged_flag=false;
    watershed_flag=false;*/

    //bluePlane_flag=false;
    //redPlane_flag=false;
    //greenPlane_flag=false;
    //RG_flag=false;
    //GR_flag=false;
    //BY_flag=false;
    //noOpponencies_flag=true;
    //noPlanes_flag=true;
    resized_flag=false;
    //----
    //maxSalienceBlob_img=new ImageOf<PixelMono>;
    outContrastLP=new ImageOf<PixelMono>;
    outMeanColourLP=new ImageOf<PixelBgr>;
    
    //wModule=this;

    max_boxes = new YARPBox[3];
    //initializing the image plotted out int the drawing area
    //image_out=new ImageOf<PixelRgb>;
    _procImage=new ImageOf<PixelRgb>;
    _outputImage3=new ImageOf<PixelRgb>;
    _outputImage3Merged=new ImageOf<PixelRgb>;
    _outputImage3Cart=new ImageOf<PixelRgb>;
    _outputImage=new ImageOf<PixelMono>;

    ptr_inputImg=new ImageOf<yarp::sig::PixelRgb>; //pointer to the input image
    //ptr_inputRed=new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the red plane
    //ptr_inputGreen= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the green plane
    //ptr_inputBlue= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the blue plane
    ptr_inputImgRed=new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the red plane
    ptr_inputImgGreen= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the green plane
    ptr_inputImgBlue= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the blue plane
    //ptr_inputRG= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the R+G- colour opponency
    //ptr_inputGR= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the G+R- colour opponency
    //ptr_inputBY= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the B+Y- colour opponency
    ptr_inputImgRG= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the R+G- colour opponency
    ptr_inputImgGR= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the G+R- colour opponency
    ptr_inputImgBY= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the B+Y- colour opponency
    edges=new ImageOf<yarp::sig::PixelMono>; //pointer to the edges image
    img=new ImageOf<PixelRgb>;
    tmpImage=new ImageOf<PixelMono>;
    image_out=new ImageOf<PixelMono>;
    
    _inputImgRGS=new ImageOf<PixelMono>;
    _inputImgGRS=new ImageOf<PixelMono>;
    _inputImgBYS=new ImageOf<PixelMono>;
    
    blobFov=new ImageOf<PixelMono>;
    

    salienceBU=0.5;
    salienceTD=0.5;
    maxBLOB=4096;
    minBLOB=100;

    constantTimeGazeControl=1;
    constantTimeCentroid=1;

    targetRED=1;
    targetGREEN=1;
    targetBLUE=1;
    searchRG=0;
    searchGR=0;
    searchBY=0;
}

blobFinderThread::blobFinderThread(int rateThread):RateThread(rateThread)
{
    ippSetNumThreads(1);

    reinit_flag=false;
    interrupted_flag=false;
    ct=0;
    filterSpikes_flag=false;
    count=0;
    freetorun=false;

    l2cTable=0;

    /*meanColour_flag=true;
    contrastLP_flag=false;
    blobCataloged_flag=true;
    foveaBlob_flag=false;
    colorVQ_flag=false;
    blobList_flag=false;
    maxSaliencyBlob_flag=false;
    tagged_flag=false;
    watershed_flag=false;*/

    //bluePlane_flag=false;
    //redPlane_flag=false;
    //greenPlane_flag=false;
    //RG_flag=false;
    //GR_flag=false;
    //BY_flag=false;
    //noOpponencies_flag=true;
    //noPlanes_flag=true;
    resized_flag=false;
    //----
    //maxSalienceBlob_img=new ImageOf<PixelMono>;
    outContrastLP=new ImageOf<PixelMono>;
    outMeanColourLP=new ImageOf<PixelBgr>;
    
    //wModule=this;

    max_boxes = new YARPBox[3];
    //initializing the image plotted out int the drawing area
    //image_out=new ImageOf<PixelRgb>;
    _procImage=new ImageOf<PixelRgb>;
    _outputImage3=new ImageOf<PixelRgb>;
    _outputImage3Merged=new ImageOf<PixelRgb>;
    _outputImage3Cart=new ImageOf<PixelRgb>;
    _outputImage=new ImageOf<PixelMono>;

    ptr_inputImg=new ImageOf<yarp::sig::PixelRgb>; //pointer to the input image
    //ptr_inputRed=new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the red plane
    //ptr_inputGreen= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the green plane
    //ptr_inputBlue= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the blue plane
    ptr_inputImgRed=new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the red plane
    ptr_inputImgGreen= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the green plane
    ptr_inputImgBlue= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the blue plane
    //ptr_inputRG= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the R+G- colour opponency
    //ptr_inputGR= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the G+R- colour opponency
    //ptr_inputBY= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the B+Y- colour opponency
    ptr_inputImgRG= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the R+G- colour opponency
    ptr_inputImgGR= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the G+R- colour opponency
    ptr_inputImgBY= new ImageOf<yarp::sig::PixelMono>; //pointer to the input image of the B+Y- colour opponency
    edges=new ImageOf<yarp::sig::PixelMono>; //pointer to the edges image
    img=new ImageOf<PixelRgb>;
    tmpImage=new ImageOf<PixelMono>;
    image_out=new ImageOf<PixelMono>;
    
    _inputImgRGS=new ImageOf<PixelMono>;
    _inputImgGRS=new ImageOf<PixelMono>;
    _inputImgBYS=new ImageOf<PixelMono>;
    
    blobFov=new ImageOf<PixelMono>;
    

    salienceBU=0.5;
    salienceTD=0.5;
    maxBLOB=4096;
    minBLOB=100;

    constantTimeGazeControl=1;
    constantTimeCentroid=1;

    targetRED=1;
    targetGREEN=1;
    targetBLUE=1;
    searchRG=0;
    searchGR=0;
    searchBY=0;
    minBoundingArea=225;

}


void blobFinderThread::setName(std::string str){
    this->name=str; 
}


std::string blobFinderThread::getName(const char* p) {
    string str(name);
    str.append(p);
    printf("name: %s", name.c_str());
    return str;
}

bool blobFinderThread::freeLookupTables() {
        if (l2cTable)
            RCdeAllocateL2CTable(l2cTable);
        //l2cTable = 0;
        return true;
    }

void blobFinderThread::reinitialise(int width, int height) {
    this->width=width;
    this->height=height;

    img->resize(width,height);
    edges->resize(width,height);
    tmpImage->resize(width,height);
    image_out->resize(width,height);
    
    resizeImages(width,height);

    //save look-up table to file
    int nEcc,nAng;
    nEcc=height;
    nAng=width;

    if (l2cTable == 0) {
        l2cTable = new lp2CartPixel[480*480];
        if (l2cTable == 0) {
            fprintf(stderr, "logPolarLibrary: can't allocate l2c lookup tables, wrong size?\n");
        }
    }

    float scaleFactor = (float) RCcomputeScaleFactor (nEcc,nAng,defaultSize,defaultSize, 1.0 );
    string path("./");
    char* ppath=(char*)path.c_str();
    RCbuildL2CMap (nEcc, nAng, defaultSize, defaultSize, 1.0,scaleFactor , 0, 0, ELLIPTICAL, ppath);
    //allocate logpolar to cartesian look-up table
    RCallocateL2CTable (l2cTable, defaultSize,defaultSize, ppath);
}


void blobFinderThread::resizeImages(int width, int height) {
    srcsize.height=height;
    srcsize.width=width;

    //int widthstep=256;
    double divider=ceil(width/32.0);
    int widthstep=divider*32;

    ptr_tagged = new yarp::sig::ImageOf<yarp::sig::PixelInt>;
    ptr_tagged->resize(widthstep,height);
    
    this->wOperator=new WatershedOperator(true,width,height,widthstep,10);
    //_wOperator=this->wOperator;
    this->salience=new SalienceOperator(width,height);
    //_salience=this->salience;

    //maxSalienceBlob_img->resize(width,height);
    outMeanColourLP->resize(width,height);
    outContrastLP->resize(width,height);

    //image_out->resize(width,height);
    _procImage->resize(width,height);
    _outputImage->resize(width,height);
    _outputImage3->resize(width,height);
    _outputImage3Merged->resize(width,height);
    _outputImage3Cart->resize(defaultSize,defaultSize);

    //ptr_inputRed->resize(width,height);
    //ptr_inputGreen->resize(width,height);
    //ptr_inputBlue->resize(width,height);
    //ptr_inputRG->resize(width,height);
    //ptr_inputGR->resize(width,height);
    //ptr_inputBY->resize(width,height);

    _inputImgRGS->resize(width,height);
    _inputImgGRS->resize(width,height);
    _inputImgBYS->resize(width,height);
    blobFov->resize(width,height);

    blobList = new char [width*height+1];

    ptr_inputImg->resize(width,height);
    _inputImgRed.resize(width,height);
    _inputImgGreen.resize(width,height);
    _inputImgBlue.resize(width,height);
    _inputImgRG.resize(width,height);
    _inputImgGR.resize(width,height);
    _inputImgBY.resize(width,height);

    resized_flag=true;
}



/**
*initialization of the thread 
*/
bool blobFinderThread::threadInit(){

    bool ret = false;
    bool ok=true;
    startTimer = Time::now();
    
    //ConstString portName2 = options.check("name",Value("/worker2")).asString();
    inputPort.open(getName("/image:i").c_str());
    edgesPort.open(getName("/edges:i").c_str());
    checkPort.open(getName("/check:o").c_str());

    rgPort.open(getName("/rg:i").c_str());
    grPort.open(getName("/gr:i").c_str());
    byPort.open(getName("/by:i").c_str());

    outputPort.open(getName("/image:o").c_str());
    outputPort3.open(getName("/imageC3:o").c_str());
    centroidPort.open(getName("/centroid:o").c_str());
    triangulationPort.open(getName("/triangulation:o").c_str());
    gazeControlPort.open(getName("/gazeControl:o").c_str());

    

    return true;
}

void blobFinderThread::resetFlags(){
    contrastLP_flag=false;
    meanColour_flag=false;
    blobCataloged_flag=false;
    foveaBlob_flag=false;
    colorVQ_flag=false;
    maxSaliencyBlob_flag=false;
    blobList_flag=false;
    tagged_flag=false;
    watershed_flag=false;
    
}

/**
* function called when the module is poked with an interrupt command
*/
void blobFinderThread::interrupt(){
    interrupted_flag=true; //this flag must be switched before the unlock of every input port

    inputPort.interrupt();//(getName("image:i"));
    edgesPort.interrupt();//getName(edges:i);
    checkPort.interrupt();

    rgPort.interrupt();//open(getName("rg:i"));
    grPort.interrupt();//open(getName("gr:i"));
    byPort.interrupt();//open(getName("by:i"));

    outputPort.interrupt();//open(getName("image:o"));
    outputPort3.interrupt();
    centroidPort.interrupt();//open(getName("centroid:o"));
    triangulationPort.interrupt();//open(getName("triangulation:o"));
    gazeControlPort.interrupt();//open(getName("gazeControl:o"));

}

/**
* active loop of the thread
*/
void blobFinderThread::run() {
    if(!interrupted_flag){
        ct++;
        //reading input port and extracting colour planes
        img = inputPort.read(false);
        if(0!=img){
            if(!reinit_flag) {
                srcsize.height=img->height();
                srcsize.width=img->width();
                this->height=img->height();
                this->width=img->width();
                reinitialise(img->width(), img->height());
                reinit_flag=true;
                img->resize(this->width,this->height);    
            }

            //copy the inputImg into a buffer
            //ippiCopy_8u_C3R(img->getRawImage(), img->getRowSize(),tmpImage->getRawImage(), tmpImage->getRowSize(),srcsize);
            ippiCopy_8u_C3R(img->getRawImage(), img->getRowSize(),ptr_inputImg->getRawImage(), ptr_inputImg->getRowSize(),srcsize);
            bool ret1=true,ret2=true;
            ret1=getOpponencies();
            ret1=true;
            ret2=getPlanes(img);
            if(ret1&&ret2)
                freetorun=true;
            if(!freetorun)
                return;

            //meanColour_flag=false;
            //blobCataloged_flag=true;

            bool redPlane_flag=false;
            bool greenPlane_flag=false;
            bool bluePlane_flag=false;
            bool RG_flag=false;
            bool GR_flag=false;
            bool BY_flag=false;
            

            

            bool conversion=true;
            //_outputImage is the single channel image of the edges _outputImage=edges
            //_outputImage=wOperator->getPlane(ptr_inputImg);
            tmpImage=edgesPort.read(false);
            if(tmpImage!=NULL)
                ippiCopy_8u_C1R(tmpImage->getRawImage(),tmpImage->getRowSize(),edges->getRawImage(), edges->getRowSize(),srcsize);
            //rain function uses as source image the _outputImage
            rain(edges);


            /*
            if(redPlane_flag){
                ippiCopy_8u_C1R(this->ptr_inputImgRed->getRawImage(),this->ptr_inputImgRed->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(greenPlane_flag){
                ippiCopy_8u_C1R(this->ptr_inputImgGreen->getRawImage(),this->ptr_inputImgGreen->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(bluePlane_flag){
                ippiCopy_8u_C1R(this->ptr_inputImgBlue->getRawImage(),this->ptr_inputImgBlue->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(RG_flag){
                ippiCopy_8u_C1R(this->ptr_inputImgRG->getRawImage(),this->ptr_inputImgRG->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(GR_flag){
                ippiCopy_8u_C1R(this->ptr_inputImgGR->getRawImage(),this->ptr_inputImgGR->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(BY_flag){
                ippiCopy_8u_C1R(this->ptr_inputImgBY->getRawImage(),this->ptr_inputImgBY->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }

            else if(this->foveaBlob_flag){
                this->salience->drawFoveaBlob(*this->salience->foveaBlob,*this->tagged);
                ippiCopy_8u_C1R(this->salience->foveaBlob->getRawImage(),this->salience->foveaBlob->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(this->watershed_flag){
                ippiCopy_8u_C1R(this->wOperator->tSrc.getRawImage(),this->wOperator->tSrc.getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(this->tagged_flag){
                //printf("dimension of the tagged image %d,%d \n", this->tagged->width(), this->tagged->height());
                for(int y=0; y<this->tagged->height(); y++){
                    for (int x=0; x<this->tagged->width(); x++){
                        int imgvalue=*tagged->getPixelAddress(x,y);
                        int value=(int)ceil(((double)max_tag/255)*imgvalue);
                        _outputImage->pixel(x,y)=value;
                    }
                }
                conversion=true;
            }
            else if(this->blobList_flag){
                this->drawAllBlobs(true);
                ippiCopy_8u_C1R((unsigned char*)this->blobList,320,_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            //else if(this->blobCataloged_flag){
            //    if(this->contrastLP_flag){
            //        this->drawAllBlobs(true);
            //        ippiCopy_8u_C1R(this->outContrastLP->getRawImage(),this->outContrastLP->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
            //        conversion=true;
            //    }
            else if(this->meanColour_flag){
                //_outputImage=_wOperator->getPlane(&_inputImg); 
                //rain();
                this->drawAllBlobs(true);
                salience->ComputeMeanColors(max_tag); //compute for every box the mean Red,Green and Blue Color.
                salience->DrawMeanColorsLP(*outMeanColourLP,*tagged);
                ippiCopy_8u_C3R(this->outMeanColourLP->getRawImage(),this->outMeanColourLP->getRowSize(),_outputImage3->getRawImage(),_outputImage3->getRowSize(),srcsize);	
                conversion=false;
                if(maxSaliencyBlob_flag) {
                    this->salience->DrawMaxSaliencyBlob(*salience->maxSalienceBlob_img,max_tag,*tagged);
                    ippiCopy_8u_C1R(salience->maxSalienceBlob_img->getRawImage(),salience->maxSalienceBlob_img->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                }
                else if(contrastLP_flag) {
                    ippiCopy_8u_C1R(this->outContrastLP->getRawImage(),this->outContrastLP->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                }
            }
                //else{
                //    _outputImage=wOperator->getPlane(ptr_inputImg); //the input is a RGB image, whereas the watershed is working with a mono image
                //    conversion=true;
                }
            //}
            else if(this->maxSaliencyBlob_flag){
                this->drawAllBlobs(true);
                this->salience->DrawMaxSaliencyBlob(*salience->maxSalienceBlob_img,max_tag,*tagged);
                ippiCopy_8u_C1R(salience->maxSalienceBlob_img->getRawImage(),salience->maxSalienceBlob_img->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(this->contrastLP_flag){
                this->drawAllBlobs(true);
                //salience->DrawMaxSaliencyBlob(*this->salience->maxSalienceBlob_img,this->max_tag,*this->tagged);
                ippiCopy_8u_C1R(this->outContrastLP->getRawImage(),this->outContrastLP->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
                conversion=true;
            }
            else if(this->colorVQ_flag){
                this->salience->DrawVQColor(*this->salience->colorVQ_img,*this->tagged);
                ippiCopy_8u_C3R(this->salience->colorVQ_img->getRawImage(),this->salience->colorVQ_img->getRowSize(),_outputImage3->getRawImage(),_outputImage3->getRowSize(),srcsize);
                conversion=false;
            }
            
            else{
                //_outputImage=wOperator->getPlane(ptr_inputImg); //the input is a RGB image, whereas the watershed is working with a mono image
                conversion=true;
            }*/

            this->drawAllBlobs(true);
            this->salience->DrawMaxSaliencyBlob(*salience->maxSalienceBlob_img,max_tag,*tagged);
            ippiCopy_8u_C1R(salience->maxSalienceBlob_img->getRawImage(),salience->maxSalienceBlob_img->getRowSize(),_outputImage->getRawImage(),_outputImage->getRowSize(),srcsize);
            salience->ComputeMeanColors(max_tag); //compute for every box the mean Red,Green and Blue Color.
            salience->DrawMeanColorsLP(*outMeanColourLP,*tagged);
            ippiCopy_8u_C3R(this->outMeanColourLP->getRawImage(),this->outMeanColourLP->getRowSize(),_outputImage3->getRawImage(),_outputImage3->getRowSize(),srcsize);	

            outPorts();

            
            if(checkPort.getOutputCount()){
                if (meanColour_flag) {
                    ippiAdd_8u_C3RSfs(ptr_inputImg->getRawImage(),ptr_inputImg->getRowSize(),_outputImage3->getRawImage(),_outputImage3->getRowSize(),_outputImage3Merged->getRawImage(),_outputImage3Merged->getRowSize(),srcsize,1);
                }
                else {
                    Ipp8u* im_tmp[3]={_outputImage->getRawImage(),_outputImage->getRawImage(),_outputImage->getRawImage()};
                    ippiCopy_8u_P3C3R(im_tmp,_outputImage->getRowSize(),img->getRawImage(),img->getRowSize(),srcsize);
                    ippiAdd_8u_C3RSfs(ptr_inputImg->getRawImage(),ptr_inputImg->getRowSize(),img->getRawImage(),img->getRowSize(),_outputImage3Merged->getRawImage(),_outputImage3Merged->getRowSize(),srcsize,1);
                }
                    
                logpolarToCart(*_outputImage3Cart,*_outputImage3Merged,l2cTable);
                checkPort.prepare() = *(_outputImage3Cart);
                checkPort.write();
            }
            

            /*endTimer=Time::now();
            double dif = endTimer-startTimer;
            startTimer=endTimer;
            printf("elapsed time in sec: %f\n", dif);*/
        }
    } //if (!interrupted)
}



void blobFinderThread::stop(){
    printf("R+G- colourOpponency port closing .... \n");
    rgPort.close();
    printf("G+R- colourOpponency port closing .... \n");
    grPort.close();
    printf("B+Y- colourOpponency port closing .... \n");
    byPort.close();

    printf("closing outputport .... \n");
    outputPort.close();
    outputPort3.close();
    checkPort.close();

    printf("input port closing .... \n");
    inputPort.close();

    printf("closing communication ports .... \n");
    centroidPort.close();
    gazeControlPort.close();
    triangulationPort.close();
    threadRelease();
}

/**
*	releases the thread
*/
void blobFinderThread::threadRelease(){
    //freeing look-up tables
    freeLookupTables();

    //deleting allocated objects
    delete outContrastLP;
    delete outMeanColourLP;

    delete _procImage;
    delete _outputImage3;
    delete _outputImage3Merged;
    delete _outputImage3Cart;
    delete _outputImage;

    delete ptr_inputImg; //pointer to the input image
    delete edges; //pointer to the edges image

    delete ptr_inputImgRed; //pointer to the input image of the red plane
    delete ptr_inputImgGreen; //pointer to the input image of the green plane
    delete ptr_inputImgBlue; //pointer to the input image of the blue plane
    delete ptr_inputImgRG; //pointer to the input image of the R+G- colour opponency
    delete ptr_inputImgGR; //pointer to the input image of the G+R- colour opponency
    delete ptr_inputImgBY; //pointer to the input image of the B+Y- colour opponency

    delete _inputImgRGS;
    delete _inputImgGRS;
    delete _inputImgBYS;

    delete blobFov;
}

/**
* function that reads the ports for colour RGB opponency maps
*/
bool blobFinderThread::getOpponencies(){
    
    tmpImage=rgPort.read(false);
    if(tmpImage!=NULL)
        ippiCopy_8u_C1R(tmpImage->getRawImage(),tmpImage->getRowSize(),ptr_inputImgRG->getRawImage(), ptr_inputImgRG->getRowSize(),srcsize);
    
    tmpImage=grPort.read(false);
    if(tmpImage!=NULL)
        ippiCopy_8u_C1R(tmpImage->getRawImage(),tmpImage->getRowSize(),ptr_inputImgGR->getRawImage(), ptr_inputImgGR->getRowSize(),srcsize);
    
    tmpImage=byPort.read(false);
    if(tmpImage!=NULL)
        ippiCopy_8u_C1R(tmpImage->getRawImage(),tmpImage->getRowSize(),ptr_inputImgBY->getRawImage(), ptr_inputImgBY->getRowSize(),srcsize);

    
    /*
    ptr_inputImgRG=rgPort.read(false);
    if(ptr_inputImgRG==0)
        return false;
    ptr_inputImgGR=grPort.read(false);
    if(ptr_inputImgGR==0)
        return false;
    ptr_inputImgBY=byPort.read(false); 
    if(ptr_inputImgBY==0)
        return false;
        */
    return true;
}

/**
* function that reads the ports for the RGB planes
*/
bool blobFinderThread::getPlanes(ImageOf<PixelRgb>* inputImage){
    /*
    tmpImage=redPort.read(true);
    if(tmpImage!=NULL)
        ippiCopy_8u_C1R(tmpImage->getRawImage(),tmpImage->getRowSize(),ptr_inputImgRed->getRawImage(), ptr_inputImgRed->getRowSize(),srcsize);
   
    tmpImage=greenPort.read(true);
    if(tmpImage!=NULL)
        ippiCopy_8u_C1R(tmpImage->getRawImage(),tmpImage->getRowSize(),ptr_inputImgGreen->getRawImage(), ptr_inputImgGreen->getRowSize(),srcsize);
    
    tmpImage=bluePort.read(true);
    if(tmpImage!=NULL)
        ippiCopy_8u_C1R(tmpImage->getRawImage(),tmpImage->getRowSize(),ptr_inputImgBlue->getRawImage(), ptr_inputImgBlue->getRowSize(),srcsize);
    */

    Ipp8u* shift[3];
    int psb;
    shift[0]=ptr_inputImgRed->getRawImage(); 
    shift[1]=ptr_inputImgGreen->getRawImage();
    shift[2]=ptr_inputImgBlue->getRawImage();
    ippiCopy_8u_C3P3R(inputImage->getRawImage(),inputImage->getRowSize(),shift,ptr_inputImgRed->getRowSize(),srcsize);
    //ippiAdd_8u_C1RSfs(redPlane->getRawImage(),redPlane->getRowSize(),greenPlane->getRawImage(),greenPlane->getRowSize(),yellowPlane->getRawImage(),yellowPlane->getRowSize(),srcsize,1);

    return true;
}


bool blobFinderThread::outPorts(){ 
    if((0!=_outputImage)&&(outputPort.getOutputCount())){ 
        outputPort.prepare() = *(_outputImage);
        outputPort.write();
    }
    if((0!=_outputImage3)&&(outputPort3.getOutputCount())){ 
        outputPort3.prepare() = *(_outputImage3);
        outputPort3.write();
    }

    /*
    if(triangulationPort.getOutputCount()){
        Bottle in,bot;
        //Bottle &bot = triangulationPort.prepare(); 
        bot.clear();
        
        bot.addString("get");
        bot.addString("3dpoint");
        bot.addString("right");
        bot.addDouble(blobFinder->salience->target_x);
        bot.addDouble(_logpolarParams::_ysize-blobFinder->salience->target_y);
        /*bot.addDouble(blobFinder->salience->centroid_x);
        bot.addDouble(_logpolarParams::_ysize-blobFinder->salience->centroid_y);*//*
       
        bot.addDouble(1.5); //fixed distance in which the saccade takes place
        triangulationPort.write(bot,in); //stop here till it receives a response
        if (in.size()>0) {
            target_z=in.pop().asDouble();
            target_y=in.pop().asDouble()+0.097;
            target_x=in.pop().asDouble();
            
        } else { 
            printf("No response\n");
        }
        bot.clear();
    }
    */

    /*
    if(gazeControlPort.getOutputCount()){
        if(!this->timeControl_flag){
            Bottle &bot = gazeControlPort.prepare(); 
            bot.clear();
            int target_xmap,target_ymap,target_zmap;
            
            bot.addDouble(target_x);  
            bot.addDouble(target_y); 
            bot.addDouble(target_z);
            gazeControlPort.writeStrict();
        }
        else{
            time (&endTimer);
            double dif = difftime (endTimer,startTimer);
            if(dif>blobFinder->constantTimeGazeControl+0.5){
                //restart the time intervall
                 time(&startTimer);
            }
            else if((dif>blobFinder->constantTimeGazeControl)&&(dif<blobFinder->constantTimeGazeControl+0.5)){
                //output the command
                //finds the entries with a greater number of occurencies 
                std::map<const char*,int>::iterator iterMap;
                /*int previousValue=occurencesMap.begin()->second;
                std::string finalKey("");
                iterMap=occurencesMap.begin();
                for(;iterMap==occurencesMap.end();iterMap++){
                    if(iterMap->second>previousValue){
                        sprintf((char*)finalKey.c_str(),"%s",iterMap->first);
                    }
                }
                //estracts the strings of the target
                size_t found;
                string target_xmap_string("");
                string target_ymap_string("");
                string target_zmap_string("");
                string rest("");

                found=finalKey.find(",");
                target_xmap_string=finalKey.substr(0,found);
                rest=finalKey.substr(found,finalKey.length()-found);
                found=finalKey.find(",");
                target_ymap_string=rest.substr(0,found);
                rest=finalKey.substr(found,finalKey.length()-found);
                found=finalKey.find(",");
                target_zmap_string=rest.substr(0,finalKey.length());*//*
                
                //subdived the string into x,y,z
                //send the command.
                Bottle &bot = gazeControlPort.prepare(); 
                bot.clear();
                int target_xmap,target_ymap, target_zmap;
                bot.addDouble(target_x);  
                bot.addDouble(target_y); 
                bot.addDouble(target_z);
                gazeControlPort.writeStrict();
                time(&startTimer);
                //clear the map
            }
            else{
                //idle period
                //check if it is present and update the map
                //std::string positionName(" ");
                /*sprintf((char*)positionName.c_str(),"%f,%f,%f",target_x,target_y,target_z);
                printf((char*)positionName.c_str());*/
                /*std::map<const char*,int>::iterator iterMap;
                
                iterMap=occurencesMap.find(positionName.c_str());

                if(iterMap==0){
                    printf("new occurence!");
                }
                else{
                    iterMap->second++;
                }*//*

            }
        }
    }*/

    /*
    if(centroidPort.getOutputCount()){
        Bottle &bot = centroidPort.prepare(); 
        bot.clear();
        
        
        
        
        time (&endTimer);
        double dif = difftime (endTimer,startTimer);
        if((dif>blobFinder->constantTimeCentroid)&&(dif<=blobFinder->constantTimeCentroid+0.5)){
            if((blobFinder->salience->target_x<previous_target_x+5)&&(blobFinder->salience->target_x>previous_target_x-5)){
                if((blobFinder->salience->target_y<previous_target_y+5)&&(blobFinder->salience->target_y>previous_target_y-5)){
                    //printf("same position \n");
                }
                else{
                    //printf("."); 
                    bot.addVocab( Vocab::encode("sac") ); 
                    bot.addVocab( Vocab::encode("img") ); 
                    double centroidDisplacementY=1.0;
                    double xrel=(blobFinder->salience->target_x-_logpolarParams::_xsize/2+xdisp)/(_logpolarParams::_xsize/2);
                    double yrel=(blobFinder->salience->target_y-_logpolarParams::_ysize/2+ydisp)/(-_logpolarParams::_ysize/2);
                    //printf("%f>%f,%f \n",dif,xrel,yrel);
                    bot.addDouble(xrel);  
                    bot.addDouble(yrel); 
                    centroidPort.write();

                    previous_target_x=blobFinder->salience->target_x;
                    previous_target_y=blobFinder->salience->target_y;
                }
            }
            else{
                //printf(".");
                bot.addVocab( Vocab::encode("sac") ); 
                bot.addVocab( Vocab::encode("img") ); 
                double centroidDisplacementY=1.0;
                double xrel=(blobFinder->salience->target_x-_logpolarParams::_xsize/2)/(_logpolarParams::_xsize/2);
                double yrel=(blobFinder->salience->target_y-_logpolarParams::_ysize/2)/(-_logpolarParams::_ysize/2);
                //printf("%f>%f,%f \n",dif,xrel,yrel);
                bot.addDouble(xrel);  
                bot.addDouble(yrel); 
                centroidPort.write();

                previous_target_x=blobFinder->salience->target_x;
                previous_target_y=blobFinder->salience->target_y;
            }
            


            /*bot.addVocab( Vocab::encode("sac") ); 
            bot.addVocab( Vocab::encode("img") ); 
            double centroidDisplacementY=1.0;
            double xrel=(blobFinder->salience->target_x-_logpolarParams::_xsize/2)/(_logpolarParams::_xsize/2);
            double yrel=(blobFinder->salience->target_y-_logpolarParams::_ysize/2)/(-_logpolarParams::_ysize/2);
            //printf("%f>%f,%f \n",dif,xrel,yrel);
            bot.addDouble(xrel);  
            bot.addDouble(yrel); 
            centroidPort.write();*//*
            
        }
        else if(dif>blobFinder->constantTimeCentroid+0.5){
            time (&startTimer);
        }
        else{
           
        }
        
    }*/

     /*Bottle& _outBottle=_centroidPort->prepare();
     _outBottle.clear();
    //_outBottle.addString("centroid:");
    _outBottle.addInt(this->sasalience->centroid_x);
    _outBottle.addInt(this->salience->centroid_y);
    _outBottle.addInt(this->salience->centroid_x);
    _outBottle.addInt(this->salience->centroid_y);
    _centroidPort->writeStrict();*/

    return true;
}

/**
* applies the watershed (rain falling) algorithm
*/
void blobFinderThread::rain(ImageOf<PixelMono>* edgesImage){
    max_tag=wOperator->apply(*edgesImage,_tagged);
    //printf("MAX_TAG=%d",wModule->max_tag);
    
    
    /*
    int psb32s;
    IppiSize srcsize={this->width,this->height};
    Ipp32s* _inputImgRGS32=ippiMalloc_32s_C1(this->width,this->height,&psb32s);
    Ipp32s* _inputImgGRS32=ippiMalloc_32s_C1(this->width,this->height,&psb32s);
    Ipp32s* _inputImgBYS32=ippiMalloc_32s_C1(this->width,this->height,&psb32s);
    
    if(ptr_inputImgRG!=0){
        ippiScale_8u32s_C1R(_inputImgRG.getRawImage(),_inputImgRG.getRowSize(),_inputImgRGS32,psb32s,srcsize);
        ippiConvert_32s8s_C1R(_inputImgRGS32,psb32s,(Ipp8s*)_inputImgRGS->getRawImage(),_inputImgRGS->getRowSize(),srcsize);
        //_inputImgRGS->copy(_inputImgRG,320,240);
    }
    else
        return;
    if(ptr_inputImgGR!=0){
        ippiScale_8u32s_C1R(_inputImgGR.getRawImage(),_inputImgGR.getRowSize(),_inputImgGRS32,psb32s,srcsize);
        ippiConvert_32s8s_C1R(_inputImgGRS32,psb32s,(Ipp8s*)_inputImgGRS->getRawImage(),_inputImgGRS->getRowSize(),srcsize);
        //_inputImgGRS->copy(_inputImgGR,320,240);
    }
    else
        return;
    if(ptr_inputImgBY!=0){
        ippiScale_8u32s_C1R(_inputImgBY.getRawImage(),_inputImgBY.getRowSize(),_inputImgBYS32,psb32s,srcsize);
        ippiConvert_32s8s_C1R(_inputImgBYS32,psb32s,(Ipp8s*)_inputImgBYS->getRawImage(),_inputImgBYS->getRowSize(),srcsize);
        //_inputImgBYS->copy(_inputImgBY,320,240);
    }
    else
        return;
        */

    salience->blobCatalog(_tagged, _inputImgRG, _inputImgGR, _inputImgBY,
        _inputImgBlue, _inputImgGreen, _inputImgRed, max_tag);
        

    blobCataloged_flag=true;
    //istruction to set the ptr_tagged in the Watershed Module with the static variable _tagged
    tagged=ptr_tagged; //ptr_tagged is the pointer to _tagged

    /*
    ippiFree(_inputImgRGS32); //Ipp32s* _inputImgRGS32=ippiMalloc_32s_C1(320,240,&psb32s);
    ippiFree(_inputImgGRS32); //Ipp32s* _inputImgGRS32=ippiMalloc_32s_C1(320,240,&psb32s);
    ippiFree(_inputImgBYS32); //Ipp32s* _inputImgBYS32=ippiMalloc_32s_C1(320,240,&psb32s);
    */
}

void blobFinderThread::drawAllBlobs(bool stable)
{
    salience->ComputeSalienceAll(this->max_tag,this->max_tag);
    //extracts the PixelBgr color of a fovea blob
    PixelBgr varFoveaBlob = salience->varBlob(*tagged, *ptr_inputImgRG, *ptr_inputImgGR, *ptr_inputImgBY, 1);

    salience->drawFoveaBlob(*blobFov, *tagged);
    //__OLD//salience.drawBlobList(blobFov, tagged, blobList, max_tag, 127);
    
    //list of boolean whether is present or not a blob
    
    memset(blobList, 0, sizeof(char)*(max_tag+1));
    // - faster
    // - it considers also "lateral" pixels
    // - it doesn't add pixels iteratively
    wOperator->findNeighborhood(*tagged, 0, 0, blobList);
    salience->fuseFoveaBlob3(*tagged, blobList, varFoveaBlob,max_tag);

    // alternative method
    //__OLD//rain.fuseFoveaBlob(tagged, blobList, max_tag);
    
    //__OLD//blobList[1]=2; // so the fovea blob is eliminated by the removeBlobList
    //__OLD//salience.statBlobList(tagged, blobList, max_tag, fovBox);
    YARPBox fovBox;
    fovBox=salience->getBlobNum(1);
    //__OLD//salience.removeBlobList(blobList, max_tag);
    salience->removeFoveaBlob(*tagged);
    //__OLD//salience.updateFoveaBlob(tagged, blobList, max_tag);

    if (stable) {
        for (int i=0; i<2; i++) {
            memset(blobList, 0, sizeof(char)*(max_tag+1));
            wOperator->findNeighborhood(*tagged, 0, 0, blobList);
            //const int minBoundingArea=15*15;
            int count=salience->countSmallBlobs(*tagged, blobList, max_tag, minBoundingArea);
            //printf("Count of small blobs: %d \n",count);
            blobList[1]=0;
            salience->mergeBlobs(*tagged, blobList, max_tag, 1);
        }

        /*__OLD//while (num!=0) {
            blobList[1]=0;
            salience.mergeBlobs(tagged, blobList, max_tag, 1);
            memset(blobList, 0, sizeof(char)*max_tag);
            rain.findNeighborhood(tagged, 0, 0, blobList);
            num = salience.checkSmallBlobs(tagged, blobList, max_tag, minBoundingArea);
        }*/
    }
        
    //__OLD//salience.drawFoveaBlob(blobFov, tagged);
    //__OLD//salience.drawBlobList(blobFov, tagged, blobList, max_tag, 127);
    
    // Comment the following line to disable the elimination of non valid blob
    //salience->RemoveNonValidNoRange(max_tag, BLOB_MAXSIZE, BLOB_MINSIZE);
    salience->RemoveNonValidNoRange(max_tag,maxBLOB,minBLOB);
    
    //__OLD//salience.DrawContrastLP(rg, gr, by, tmp1, tagged, max_tag, 0, 1, 30, 42, 45); // somma coeff pos=3 somma coeff neg=-3
    //__OLD//salience.checkIOR(tagged, IORBoxes, num_IORBoxes);
    //__OLD//salience.doIOR(tagged, IORBoxes, num_IORBoxes);
    //float salienceBU=1.0,salienceTD=0.0;
    //IppiSize srcsize={this->width,this->height};
    PixelMono searchTD=0;
    PixelMono pixelRG=0,pixelGR=0,pixelBY=0;
    searchRG=((targetRED-targetGREEN+255)/510)*255;
    pixelRG=searchRG;
    //pixelRG=targetRED;
    searchGR=((targetGREEN-targetRED+255)/510)*255;
    pixelGR=searchGR;
    //pixelGR=targetGREEN;
    PixelMono addRG=((targetRED+targetGREEN)/510)*255;
    searchBY=((targetBLUE-addRG+255)/510)*255; 
    pixelBY=searchBY;
    //pixelBY=targetBLUE;
    //printf("colour: %d,%d,%d  coeff: %f,%f\n",pixelRG, pixelGR,pixelBY,salienceBU,salienceTD);

    //int psb32s;
    //int psb8u;
    //Ipp32s* _inputImgRGS32=ippiMalloc_32s_C1(this->width,this->height,&psb32s);
    //Ipp32s* _inputImgGRS32=ippiMalloc_32s_C1(this->width,this->height,&psb32s);
    //Ipp32s* _inputImgBYS32=ippiMalloc_32s_C1(this->width,this->height,&psb32s);
    //Ipp8s* _inputImgRGS8s=ippiMalloc_8u_C1(width,height,&psb8u);

    /*
    if(ptr_inputImgRG!=NULL){
        //_inputImgRGS->copy(*ptr_inputImgRG,320,240);
        ippiScale_8u32s_C1R(_inputImgRG.getRawImage(),_inputImgRG.getRowSize(),_inputImgRGS32,psb32s,srcsize);
        //_inputImgRGS8s=(Ipp8s*)_inputImgRGS->getRawImage();
        ippiConvert_32s8s_C1R(_inputImgRGS32,psb32s,(Ipp8s*)_inputImgRGS->getRawImage(),_inputImgRGS->getRowSize(),srcsize);
        //ippiCopy_8u_C1R(_inputImgRG.getPixelAddress(0,0),320,_inputImgRGS->getPixelAddress(0,0),320,srcsize);
    }
    else
        return;
    if(ptr_inputImgGR!=NULL){
        //_inputImgGRS->copy(*ptr_inputImgGR,320,240);
        ippiScale_8u32s_C1R(_inputImgGR.getRawImage(),_inputImgGR.getRowSize(),_inputImgGRS32,psb32s,srcsize);
        ippiConvert_32s8s_C1R(_inputImgGRS32,psb32s,(Ipp8s*)_inputImgGRS->getRawImage(),_inputImgGRS->getRowSize(),srcsize);
        //ippiCopy_8u_C1R(_inputImgGR.getPixelAddress(0,0),320,_inputImgGRS->getPixelAddress(0,0),320,srcsize);
    }
    else
        return;
    if(ptr_inputImgBY!=NULL){
        //_inputImgBYS->copy(*ptr_inputImgBY,320,240);
        ippiScale_8u32s_C1R(_inputImgBY.getRawImage(),_inputImgBY.getRowSize(),_inputImgBYS32,psb32s,srcsize);
        ippiConvert_32s8s_C1R(_inputImgBYS32,psb32s,(Ipp8s*)_inputImgBYS->getRawImage(),_inputImgBYS->getRowSize(),srcsize);
        //ippiCopy_8u_C1R(_inputImgBY.getPixelAddress(0,0),320,_inputImgBYS->getPixelAddress(0,0),320,srcsize);
    }
    else
        return;
    */
    if(salienceBU<1) {
        salienceBU=0;
    }
    if(salienceTD<1) {
        salienceTD=0;
    }
    int nBlobs=salience->DrawContrastLP2(_inputImgRG, _inputImgGR, _inputImgBY,
        *outContrastLP, *tagged, max_tag,
        salienceBU, salienceTD,
        pixelRG, pixelGR, pixelBY, 255); // somma coeff pos=3 somma coeff neg=-3

    //salience->ComputeMeanColors(max_tag); //compute for every box the mean Red,Green and Blue Color.
    //salience->DrawMeanColorsLP(*outMeanColourLP,*tagged);
    
    //__OLD//meanOppCol.Zero();
    //__OLD//salience.DrawMeanOpponentColorsLP(meanOppCol, tagged);

    /*__OLD//blobFinder.DrawGrayLP(tmp1, tagged, 200);
    //__OLD//ACE_OS::sprintf(savename, "./rain.ppm");
    //__OLD//YARPImageFile::Write(savename, tmp1);*/

    

    //__OLD//rain.tags2Watershed(tagged, oldWshed);

    //delete(blobList);
    //ippiFree(_inputImgRGS32); //Ipp32s* _inputImgRGS32=ippiMalloc_32s_C1(320,240,&psb32s);
    //ippiFree(_inputImgGRS32); //Ipp32s* _inputImgGRS32=ippiMalloc_32s_C1(320,240,&psb32s);
    //ippiFree(_inputImgBYS32); //Ipp32s* _inputImgBYS32=ippiMalloc_32s_C1(320,240,&psb32s);
    //ippiFree(_inputImgRGS8s);
}
