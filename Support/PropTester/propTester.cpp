/*
  OFX Support Library, a library that skins the OFX plug-in API with C++ classes.
  Copyright (C) 2004 The Foundry Visionmongers Ltd
  Author Bruno Nicoletti bruno@thefoundry.co.uk

  This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  The Foundry Visionmongers Ltd
  35-36 Gt Marlborough St 
  London W1V 7FN
  England
*/

#include <stdio.h>
#include "ofxsImageEffect.H"

////////////////////////////////////////////////////////////////////////////////
/** @brief base class of the plugin */
class BasePlugin : public OFX::ImageEffect {
protected :
  // do not need to delete this, the ImageEffect is managing them for us
  OFX::Clip *dstClip_;

public :
  /** @brief ctor */
  BasePlugin(OfxImageEffectHandle handle)
    : ImageEffect(handle)
    , dstClip_(0)
  {
    dstClip_ = fetchClip("Output");
  }

};

////////////////////////////////////////////////////////////////////////////////
/** @brief generator effect version of the plugin */
class GeneratorPlugin : public BasePlugin {
public :
  /** @brief ctor */
  GeneratorPlugin(OfxImageEffectHandle handle)
    : BasePlugin(handle)
  {}

  /** @brief client render function, this is one of the few that must be set */
  virtual void render(const OFX::RenderArguments &args);
};

////////////////////////////////////////////////////////////////////////////////
/** @brief filter version of the plugin */
class FilterPlugin : public BasePlugin {
protected :
  // do not need to delete this, the ImageEffect is managing them for us
  OFX::Clip *srcClip_;

public :
  /** @brief ctor */
  FilterPlugin(OfxImageEffectHandle handle)
    : BasePlugin(handle)
    , srcClip_(0)
  {
    srcClip_ = fetchClip("Source");
  }

  /** @brief client render function, this is one of the few that must be set */
  virtual void render(const OFX::RenderArguments &args);
};


/** @brief OFX namespace */
namespace OFX {

  /** @brief plugin namespace */
  namespace Plugin {
    
    /** @brief identify the plug-in */
    void getPluginID(OFX::PluginID &id)
    {
      id.pluginIdentifier = "co.uk.thefoundry:propertyTester";
      id.pluginVersionMajor = 1;
      id.pluginVersionMinor = 0;      
    }

    /** @brief empty load function */
    void loadAction(void)
    {
      // do anything here that you need to do globally
      fprintf(stderr, "load called!\n");
    }

    /** brief empty unload function */
    void unloadAction(void)    {
      // do anything here that you need to undo globally
      fprintf(stderr, "unload called!\n");
    }

    /** @brief The basic describe function, passed a plugin descriptor */
    void describe(OFX::ImageEffectDescriptor &desc) 
    {
      // basic labels
      desc.setLabels("Prop Tester", "Prop Tester", "Property Tester");
      desc.setPluginGrouping("OFX Test");

      // add the supported contexts, only filter at the moment
      desc.addSupportedContext(eContextGenerator);
      desc.addSupportedContext(eContextFilter);

      // add supported pixel depths
      desc.addSupportedBitDepth(eBitDepthUByte);
      desc.addSupportedBitDepth(eBitDepthUShort);
      desc.addSupportedBitDepth(eBitDepthFloat);

      // set a few flags
      desc.setSingleInstance(false);
      desc.setHostFrameThreading(false);
      desc.setSupportsMultiResolution(true);
      desc.setSupportsTiles(true);
      desc.setTemporalClipAccess(false);
      desc.setRenderTwiceAlways(false);
      desc.setSupportsMultipleClipPARs(false);
    }
        
    /** @brief describe a string param with the given name and type */
    void describeStringParam(OFX::ImageEffectDescriptor &desc, const std::string &name, StringTypeEnum strType)
    {
      StringParamDescriptor *param = desc.defineStringParam(name);
      param->setDefault(name);
      param->setScriptName(name);
      param->setHint("A string parameter");
      param->setLabels(name, name, name);
      param->setStringType(strType);
    }

    /** @brief describe a double param */
    void describeDoubleParam(OFX::ImageEffectDescriptor &desc, const std::string &name, DoubleTypeEnum doubleType,
                             double min, double max)
    {
      DoubleParamDescriptor *param = desc.defineDoubleParam(name);
      param->setLabels(name, name, name);
      param->setScriptName(name);
      param->setHint("A double parameter");
      param->setDefault(0);
      param->setRange(min, max);
      param->setDislayRange(min, max);
      param->setDoubleType(doubleType);
    }

    /** @brief describe a double param */
    void describe2DDoubleParam(OFX::ImageEffectDescriptor &desc, const std::string &name, DoubleTypeEnum doubleType,
                               double min, double max)
    {
      Double2DParamDescriptor *param = desc.defineDouble2DParam(name);
      param->setLabels(name, name, name);
      param->setScriptName(name);
      param->setHint("A 2D double parameter");
      param->setDefault(0, 0);
      param->setRange(min, min, max, max);
      param->setDislayRange(min, min, max, max);
      param->setDoubleType(doubleType);
    }

    /** @brief describe a double param */
    void describe3DDoubleParam(OFX::ImageEffectDescriptor &desc, const std::string &name, DoubleTypeEnum doubleType,
                               double min, double max)
    {
      Double3DParamDescriptor *param = desc.defineDouble3DParam(name);
      param->setLabels(name, name, name);
      param->setScriptName(name);
      param->setHint("A 3D double parameter");
      param->setDefault(0, 0, 0);
      param->setRange(min, min, min, max, max, max);
      param->setDislayRange(min, min, min, max, max, max);
      param->setDoubleType(doubleType);
    }

    /** @brief The describe in context function, passed a plugin descriptor and a context */
    void describeInContext(OFX::ImageEffectDescriptor &desc, ContextEnum context) 
    {
      // Source clip only in the filter context
      if(context == eContextFilter) {
        // create the mandated source clip
        ClipDescriptor *srcClip = desc.defineClip("Source");
        srcClip->addSupportedComponent(ePixelComponentRGBA);
        srcClip->setTemporalClipAccess(false);
        srcClip->setOptional(false);
        srcClip->setSupportsTiles(true);
        srcClip->setIsMask(false);
      }

      // create the mandated output clip
      ClipDescriptor *dstClip = desc.defineClip("Output");
      dstClip->addSupportedComponent(ePixelComponentRGBA);
      dstClip->setTemporalClipAccess(false);
      dstClip->setOptional(false);
      dstClip->setSupportsTiles(true);
      dstClip->setIsMask(false);

            
      // make some pages and to things in 
      PageParamDescriptor *page1 = desc.definePageParam("page1");
      PageParamDescriptor *page2 = desc.definePageParam("page2");
      PageParamDescriptor *page3 = desc.definePageParam("page3");

      // make an int param
      IntParamDescriptor *iParam = desc.defineIntParam("Int");
      iParam->setLabels("Int", "Int", "Int");
      iParam->setScriptName("int");
      iParam->setHint("An integer parameter");
      iParam->setDefault(0);
      iParam->setRange(-100, 100);
      iParam->setDislayRange(-100, 100);

      // make a 2D int param
      Int2DParamDescriptor *i2DParam = desc.defineInt2DParam("Int2D");
      i2DParam->setLabels("Int2D", "Int2D", "Int2D");
      i2DParam->setScriptName("int2D");
      i2DParam->setHint("A 2D integer parameter");
      i2DParam->setDefault(0, 0);
      i2DParam->setRange(-100, -100, 100, 100);
      i2DParam->setDislayRange(-100, -100, 100, 100);

      // make a 3D int param
      Int3DParamDescriptor *i3DParam = desc.defineInt3DParam("Int3D");
      i3DParam->setLabels("Int3D", "Int3D", "Int2D");
      i3DParam->setScriptName("int3D");
      i3DParam->setHint("A 3D integer parameter");
      i3DParam->setDefault(0, 0, 0);
      i3DParam->setRange(-100, -100, -100, 100, 100, 100);
      i3DParam->setDislayRange(-100, -100, -100, 100, 100, 100);

      // make a 1D double parameter of each type
      describeDoubleParam(desc, "double", eDoubleTypePlain, -100, 100);
      describeDoubleParam(desc, "angle", eDoubleTypeAngle, -100, 100);
      describeDoubleParam(desc, "scale", eDoubleTypeScale, -1, 1);
      describeDoubleParam(desc, "time", eDoubleTypeTime, -100, 100);
      describeDoubleParam(desc, "absoluteTime", eDoubleTypeAbsoluteTime, 0, 1000);
      describeDoubleParam(desc, "X_Value", eDoubleTypeNormalisedX, -1, 1);
      describeDoubleParam(desc, "Y_Value", eDoubleTypeNormalisedY, -1, 1);
      describeDoubleParam(desc, "X_Position", eDoubleTypeNormalisedXAbsolute, -1, 1);
      describeDoubleParam(desc, "Y_Position", eDoubleTypeNormalisedYAbsolute, -1, 1);

      // make a 2D double parameter of each type
      describe2DDoubleParam(desc, "double2D", eDoubleTypePlain, -100, 100);
      describe2DDoubleParam(desc, "angle2D", eDoubleTypeAngle, -100, 100);
      describe2DDoubleParam(desc, "scale2D", eDoubleTypeScale, -1, 1);
      describe2DDoubleParam(desc, "XY_Value", eDoubleTypeNormalisedXY, -1, 1);
      describe2DDoubleParam(desc, "XY_Position", eDoubleTypeNormalisedXYAbsolute, -1, 1);

      // make a 3D double parameter of each type
      describe3DDoubleParam(desc, "double3D", eDoubleTypePlain, -100, 100);
      describe3DDoubleParam(desc, "angle3D", eDoubleTypeAngle, -100, 100);
      describe3DDoubleParam(desc, "scale3D", eDoubleTypeScale, -1, 1);

      // make a string param param of each type
      describeStringParam(desc, "singleLine", eStringTypeSingleLine);
      describeStringParam(desc, "multiLine", eStringTypeMultiLine);
      describeStringParam(desc, "filePath", eStringTypeFilePath);
      describeStringParam(desc, "dirPath", eStringTypeDirectoryPath);
      describeStringParam(desc, "label", eStringTypeLabel);

      // rgba colours
      RGBAParamDescriptor *rgba = desc.defineRGBAParam("rgba");
      rgba->setLabels("rgba", "rgba", "rgba");
      rgba->setDefault(0, 0, 0, 1);
            
      // rgb colour
      RGBParamDescriptor *rgb = desc.defineRGBParam("rgb");
      rgb->setLabels("rgb", "rgb", "rgb");
      rgb->setDefault(0, 0, 0);

      // boolean
      BooleanParamDescriptor *boolean = desc.defineBooleanParam("bool");
      boolean->setLabels("bool", "bool", "bool");
      boolean->setDefault(false);

      // choice 
      ChoiceParamDescriptor *choice = desc.defineChoiceParam("bool");
      choice->setLabels("choice", "choice", "choice");
      choice->setDefault(0);
      choice->appendOption("This");
      choice->appendOption("That");
      choice->appendOption("The Other");
      choice->resetOptions();
      choice->appendOption("Tom");
      choice->appendOption("Dick");
      choice->appendOption("Harry");
            
      // push button
      PushButtonParamDescriptor *push = desc.definePushButtonParam("push");
      push->setLabels("push me", "push me", "push me Big Nose");

      // make a custom param
      CustomParamDescriptor *custom = desc.defineCustomParam("custom");
      custom->setLabels("custom", "custom", "custom");
      custom->setDefault("wibble");

    }

    /** @brief The create instance function, the plugin must return an object derived from the \ref OFX::ImageEffect class */
    ImageEffect *createInstance(OfxImageEffectHandle handle, ContextEnum context)
    {
      if(context == eContextFilter) 
        return new FilterPlugin(handle);
      else if(context == eContextGenerator) 
        return new GeneratorPlugin(handle);

      // HACK!!! Throw something here!
      return NULL; // to shut the warning up
    }
  };
};


////////////////////////////////////////////////////////////////////////////////
/** @brief render for the generator */
void
GeneratorPlugin::render(const OFX::RenderArguments &args)
{
  OFX::Image *dst = 0;

  try {
    // get a dst image
    dst = dstClip_->fetchImage(args.time);

    // push some pixels
    // blah;
    // blah;
    // blah;
  }
  
  catch(...) {
    delete dst;
    throw;
  }

  // delete them
  delete dst;
}

////////////////////////////////////////////////////////////////////////////////
/** @brief render for the filter */
void
FilterPlugin::render(const OFX::RenderArguments &args)
{
  OFX::Image *src = 0, *dst = 0;

  try {
    // get a src image
    src = srcClip_->fetchImage(args.time);

    // get a dst image
    dst = dstClip_->fetchImage(args.time);

    // push some pixels
    // blah;
    // blah;
    // blah;
  }
  
  catch(...) {
    delete src;
    delete dst;
    throw;
  }

  // delete them
  delete src;
  delete dst;
}