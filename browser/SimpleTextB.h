/**
	This snippet is direved from http://wiki.ogre3d.org/Simple+text
	Right reserved for lonewolff and other contributors
**/

//-----------------------------------------------------------------------------
// Lonewolff
//
// Filename:    SimpleText.h
// Description: Class for simple text in Ogre (Version 040507:18.30)
//-----------------------------------------------------------------------------

#include "OgreTextAreaOverlayElement.h"
#include "OgreStringConverter.h"

using namespace Ogre;

#ifndef __SimpleText_H__
#define __SimpleText_H__

class SimpleTextB
{
public:
    SimpleTextB()
    {
        olm=OverlayManager::getSingletonPtr();
        if(init==0)
        {
        panel=static_cast<OverlayContainer*>(olm->createOverlayElement("Panel","GUI"));
            panel->setMetricsMode(Ogre::GMM_PIXELS);
            panel->setPosition(0,0);
            panel->setDimensions(1.0f,1.0f);
            overlay=olm->create("GUI_OVERLAY");
            overlay->add2D(panel);
        }
        ++(this->init);
        szElement="element_"+StringConverter::toString(init);
        overlay=olm->getByName("GUI_OVERLAY");
        panel=static_cast<OverlayContainer*>(olm->getOverlayElement("GUI"));
        textArea=static_cast<TextAreaOverlayElement*>(olm->createOverlayElement("TextArea",szElement));
        panel->addChild(textArea);
        overlay->show();
    }
    ~SimpleTextB()
    {
        szElement="element_"+StringConverter::toString(init);
        olm->destroyOverlayElement(szElement);
        --(this->init);
        if(init==0)
        {
            olm->destroyOverlayElement("GUI");
            olm->destroy("GUI_OVERLAY");
        }
    }
    void setText(char *szString)
    {
        textArea->setCaption(szString);
        textArea->setDimensions(1.0f,1.0f);
        textArea->setMetricsMode(Ogre::GMM_RELATIVE);
//        textArea->setFontName("BlueHighway");
        textArea->setFontName("times");
        textArea->setCharHeight(0.03f);
    }
    void setText(String szString) // now You can use Ogre::String as text
    {
        textArea->setCaption(szString);
        textArea->setDimensions(1.0f,1.0f);
        textArea->setMetricsMode(Ogre::GMM_RELATIVE);
//        textArea->setFontName("BlueHighway");
        textArea->setFontName("times");
        textArea->setCharHeight(0.03f);
    }
    void setPos(float x,float y)
    {
        textArea->setPosition(x,y);
    }
    void setCol(float R,float G,float B,float I)
    {
        textArea->setColour(Ogre::ColourValue(R,G,B,I));
    }
private:
    OverlayManager *olm;
    OverlayContainer *panel ;
    Overlay *overlay;
    TextAreaOverlayElement *textArea;
    static int init;
    String szElement;
};

int SimpleTextB::init=0;

#endif
