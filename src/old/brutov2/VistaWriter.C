#include "VistaWriter.h"

/* global includes */
#include <iostream>
#include <fstream>
#include <ios>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>
#include "viaio/Vlib.h"
#include "viaio/VImage.h"
#include "viaio/file.h"

/* local includes */
#include "IImageDataset.h"
#include "ImageDatasetParameter.h"
#include "Image.h"
#include "ConverterConfiguration.h"
#include "Logger.h"
#include "contrast.h"
#include "convert.h"


using namespace converter;
using namespace logger;
using namespace std;

namespace vista
{

ConverterConfiguration* configuration;

VistaWriter::VistaWriter(ConverterConfiguration* cc) {
	configuration = cc;
}

VistaWriter::~VistaWriter() {
}

void VistaWriter::write(vector<IImageDataset*> datasets) {
	stringstream _dbgmsg;
	_dbgmsg << "VistaWriter: writing " << datasets.size() << " datasets";
	Logger::getInstance().debug(_dbgmsg.str());
	
	// the resulting vista images
	vector<VImage> convertedImages;
	
	
	vector<IImageDataset*>::iterator dataset = datasets.begin();
	while(dataset != datasets.end()) {
	
		string imageType = (*dataset)->getParameter(DATASET_TYPE)->getValue();
	
		if(imageType.compare("FUNCTIONAL") == 0) {
			
			Logger::getInstance().info("dataset type is FUNCTIONAL");
	
			ImageDatasetParameter* dsPar;
			
			dsPar = (*dataset)->getParameter(DIMENSION_SIZE);
			int width        = atoi(dsPar->getValue(0).c_str());
			int height       = atoi(dsPar->getValue(1).c_str());
			int nSlices      = atoi(dsPar->getValue(2).c_str());
			int nRepetitions = atoi(dsPar->getValue(3).c_str());
	
			// TODO: create a private method for doing the parameter nonsense to make
			//       make sure functional and anatomical datasets are 'in sync'
			Logger::getInstance().debug("getting PATIENT_NAME parameter");
			ImageDatasetParameter* patientNameParameter             = (*dataset)->getParameter(PATIENT_NAME);
			Logger::getInstance().debug("getting PATIENT_BIRTHDAY parameter");
			ImageDatasetParameter* patientBirthdayParameter         = (*dataset)->getParameter(PATIENT_BIRTHDAY);
			Logger::getInstance().debug("getting PATIENT_SEX parameter");
			ImageDatasetParameter* patientSexParameter              = (*dataset)->getParameter(PATIENT_SEX);
			Logger::getInstance().debug("getting MODALITY parameter");
			ImageDatasetParameter* modalityParameter                = (*dataset)->getParameter(MODALITY);
			Logger::getInstance().debug("getting DEVICE parameter");
			ImageDatasetParameter* deviceParameter                  = (*dataset)->getParameter(DEVICE);
			Logger::getInstance().debug("getting DEVICE_SOFTWARE parameter");
			ImageDatasetParameter* deviceSoftwareParameter          = (*dataset)->getParameter(DEVICE_SOFTWARE);
			Logger::getInstance().debug("getting COIL_ID parameter");
			ImageDatasetParameter* coilIDParameter                  = (*dataset)->getParameter(COIL_ID);
			Logger::getInstance().debug("getting METHOD_NAME parameter");
			ImageDatasetParameter* methodNameParameter              = (*dataset)->getParameter(METHOD_NAME);
			Logger::getInstance().debug("getting PROTOCOL_NAME parameter");
			ImageDatasetParameter* protocolNameParameter            = (*dataset)->getParameter(PROTOCOL_NAME);
			Logger::getInstance().debug("getting AQUISITION_DATE parameter");
			ImageDatasetParameter* dateParameter                    = (*dataset)->getParameter(AQUISITION_DATE);
			Logger::getInstance().debug("getting AQUISITION_TIME parameter");
			ImageDatasetParameter* timeParameter                    = (*dataset)->getParameter(AQUISITION_TIME);
			Logger::getInstance().debug("getting ECHO_TIME parameter");
			ImageDatasetParameter* echoTimeParameter                = (*dataset)->getParameter(ECHO_TIME);
			Logger::getInstance().debug("getting FIELD_OF_VIEW parameter");
			ImageDatasetParameter* fovParameter                     = (*dataset)->getParameter(FIELD_OF_VIEW);
			Logger::getInstance().debug("getting VOXEL_SIZE parameter");
			ImageDatasetParameter* voxelSizeParameter               = (*dataset)->getParameter(VOXEL_SIZE);
			Logger::getInstance().debug("getting REPETITION_TIME parameter");
			ImageDatasetParameter* repetitionTimeParameter          = (*dataset)->getParameter(REPETITION_TIME);
			Logger::getInstance().debug("getting FLIP_ANGLE parameter");
			ImageDatasetParameter* flipAngleParameter               = (*dataset)->getParameter(FLIP_ANGLE);
			Logger::getInstance().debug("getting SLICE_PACK_NUMBER_OF_SLICES parameter");
			ImageDatasetParameter* slicePackNumberOfSlicesParameter = (*dataset)->getParameter(SLICE_PACK_NUMBER_OF_SLICES);
			Logger::getInstance().debug("getting SLICE_PACK_AXES parameter");
			ImageDatasetParameter* slicePackAxesParameter           = (*dataset)->getParameter(SLICE_PACK_AXES);
			Logger::getInstance().debug("getting SLICE_PACK_POSITION parameter");
			ImageDatasetParameter* slicePackPositionParameter       = (*dataset)->getParameter(SLICE_PACK_POSITION);
			Logger::getInstance().debug("getting SLICE_PACK_SLICE_DISTANCE parameter");
			ImageDatasetParameter* slicePackSliceDistanceParameter  = (*dataset)->getParameter(SLICE_PACK_SLICE_DISTANCE);
			Logger::getInstance().debug("getting SLICE_PACK_SLICE_GAP parameter");
//			ImageDatasetParameter* slicePackSliceGapParameter       = (*dataset)->getParameter(SLICE_PACK_SLICE_GAP);
			Logger::getInstance().debug("getting SLICE_PACK_ORIENTATION parameter");
			ImageDatasetParameter* slicePackOrientationParameter    = (*dataset)->getParameter(SLICE_PACK_ORIENTATION);
			Logger::getInstance().debug("getting SLICE_TIME parameter");
			ImageDatasetParameter* sliceTimeParameter               = (*dataset)->getParameter(SLICE_TIME);
			
			for(int slice = 0; slice < nSlices; slice++) {
				VImage _image = VCreateImage(nRepetitions, height, width, VShortRepn);
				stringstream _msg;
				_msg << "converting slice " << slice;
				Logger::getInstance().info(_msg.str());
				if(slicePackOrientationParameter->getValue().compare("axial") == 0 ||
					slicePackOrientationParameter->getValue().compare("coronal") == 0 ||
					slicePackOrientationParameter->getValue().compare("transversal") == 0) {
					// need to flip the images (and change 'transversal' to 'axial'
					if(slicePackOrientationParameter->getValue().compare("transversal") == 0) {
						slicePackOrientationParameter = new ImageDatasetParameter(slicePackOrientationParameter->getName(), "axial");
					}
					for(int repetition = 0; repetition < nRepetitions; repetition++) {
						Image* repImage = (*dataset)->getImage(slice + (repetition * nSlices));
						for(int y = 0; y < height; y++) {
							for(int x = 0; x < width; x++) {
								VSetPixel(_image, repetition, y, width - x - 1, (short)repImage->getPixelValue(x, y));
							}
						}
					}
				} else {
					for(int repetition = 0; repetition < nRepetitions; repetition++) {
						Image* repImage = (*dataset)->getImage(slice + (repetition * nSlices));
						for(int y = 0; y < height; y++) {
							for(int x = 0; x < width; x++) {
								VSetPixel(_image, repetition, y, x, (short)repImage->getPixelValue(x, y));
							}
						}
					}
				}

				string outputPrecision;
				configuration->getConfigurationValue("output.precision", outputPrecision);
	
				// if precision is byte we have to 'scale' the images
				// NOTE: for functional images this is rather silly - but the
				//       user makes the rules especially the stupid ones 
				// TODO: we need to decide if we really want to use the cut-off
				//       values for functional images (it's nonsense anyway - so it
				//       shouldn't really matter that much)
				if(outputPrecision.compare("byte") == 0) {
					Logger::getInstance().debug("[VistaWriter] scaling intensity");
					// TODO: we should implement some convenience methods for getting
					//       double, int, bool, ... configuration values
					string cutOffLower, cutOffUpper;
					configuration->getConfigurationValue("histogram.cut.off.lower", cutOffLower);
					configuration->getConfigurationValue("histogram.cut.off.upper", cutOffUpper);
					_image = scaleSliceIntensity(_image, (double)atof(cutOffLower.c_str()), (double)atof(cutOffUpper.c_str()));
				}


				
				// append our parameters
				VAttrList _aList = VImageAttrList(_image);

				VAppendAttr(_aList, "bandtype", NULL, VStringRepn, "temporal");
				Logger::getInstance().debug("[VistaWriter] appending 'patient' attribute");
				string _pn = (patientNameParameter->getValue().size() == 0 ? " " : patientNameParameter->getValue());
				VAppendAttr(_aList, "patient", NULL, VStringRepn, _pn.c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'birth' attribute");
				string _pb = (patientBirthdayParameter->getValue().size() == 0 ? " " : patientBirthdayParameter->getValue());
				VAppendAttr(_aList, "birth", NULL, VStringRepn, _pb.c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'sex' attribute");
				string _ps = (patientSexParameter->getValue().size() == 0 ? " " : patientSexParameter->getValue());
				VAppendAttr(_aList, "sex", NULL, VStringRepn, _ps.c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'modality' attribute");
				VAppendAttr(_aList, "modality", NULL, VStringRepn, modalityParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'method' attribute");
				VAppendAttr(_aList, "method", NULL, VStringRepn, methodNameParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'protocol' attribute");
				VAppendAttr(_aList, "protocol", NULL, VStringRepn, protocolNameParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'device' attribute");
				VAppendAttr(_aList, "device", NULL, VStringRepn, deviceParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'device_software' attribute");
				VAppendAttr(_aList, "device_software", NULL, VStringRepn, deviceSoftwareParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'coil_id' attribute");
				VAppendAttr(_aList, "coil_id", NULL, VStringRepn, coilIDParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'date' attribute");
				VAppendAttr(_aList, "date", NULL, VStringRepn, dateParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'time' attribute");
				VAppendAttr(_aList, "time", NULL, VStringRepn, timeParameter->getValue().c_str());

				// axes				
				Logger::getInstance().debug("[VistaWriter] appending 'x-axis' attribute");
				stringstream _xass;
				_xass << slicePackAxesParameter->getValue(0) << " " << slicePackAxesParameter->getValue(3) << " " << slicePackAxesParameter->getValue(6);
				VAppendAttr(_aList, "x-axis", NULL, VStringRepn, _xass.str().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'y-axis' attribute");
				stringstream _yass;
				_yass << slicePackAxesParameter->getValue(1) << " " << slicePackAxesParameter->getValue(4) << " " << slicePackAxesParameter->getValue(7);
				VAppendAttr(_aList, "y-axis", NULL, VStringRepn, _yass.str().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'z-axis' attribute");
				stringstream _zass;
				_zass << slicePackAxesParameter->getValue(2) << " " << slicePackAxesParameter->getValue(5) << " " << slicePackAxesParameter->getValue(8);
				VAppendAttr(_aList, "z-axis", NULL, VStringRepn, _zass.str().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'convention' attribute");
				VAppendAttr(_aList, "convention", NULL, VStringRepn, "natural");
				
				// location
				Logger::getInstance().debug("[VistaWriter] appending 'location' attribute");
				float _pos     = atof(slicePackPositionParameter->getValue(0).c_str());
				float _nSlices = atof(slicePackNumberOfSlicesParameter->getValue(0).c_str());
				float _dist    = atof(slicePackSliceDistanceParameter->getValue(0).c_str());
	
				float _location = _pos - ((_nSlices - 1) * _dist * 0.5);
				VAppendAttr(_aList, "location", NULL, VFloatRepn, _location);

				Logger::getInstance().debug("[VistaWriter] appending 'fov' attribute");
				string _fovString = fovParameter->getValue(0) + " " + fovParameter->getValue(1);
				VAppendAttr(_aList, "fov", NULL, VStringRepn, _fovString.c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'voxel' attribute");
				string _voxelSizeString = voxelSizeParameter->getValue(0) + " " + voxelSizeParameter->getValue(1) + " " + voxelSizeParameter->getValue(2);
				VAppendAttr(_aList, "voxel", NULL, VStringRepn, _voxelSizeString.c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'orientation' attribute");
				VAppendAttr(_aList, "orientation", NULL, VStringRepn, slicePackOrientationParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'flip_angle' attribute");
				VAppendAttr(_aList, "flip_angle", NULL, VStringRepn, flipAngleParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'echo_time' attribute");
				VAppendAttr(_aList, "echo_time", NULL, VStringRepn, echoTimeParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'repetition_time' attribute");
				VAppendAttr(_aList, "repetition_time", NULL, VStringRepn, repetitionTimeParameter->getValue().c_str());
				Logger::getInstance().debug("[VistaWriter] appending 'slice_time' attribute");
				VAppendAttr(_aList, "slice_time", NULL, VStringRepn, sliceTimeParameter->getValue(slice).c_str());
				
				
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//
				// THIS SHOULDN'T BE HERE AT ALL !!!
				//
				// TODO: need to think about some ... function-pointer, callback, RTTI, ... something
				//       to get this out of what is supposed to be a GENERAL vista writer.
				//
				// ... grumble ... -ts.
				
				stringstream _this_is_complete_crap;
				_this_is_complete_crap << " repetition_time=" << repetitionTimeParameter->getValue() << " packed_data=1 " << nRepetitions << " ";
				VAppendAttr(_aList, "MPIL_vista_0", NULL, VStringRepn, _this_is_complete_crap.str().c_str());
				
				
				
				convertedImages.push_back(_image);
			}
			
		} else if (imageType.compare("ANATOMICAL") == 0) {
			// this is ANATOMICAL
	
			Logger::getInstance().info("dataset type is ANATOMICAL");
	
			ImageDatasetParameter* dsPar = (*dataset)->getParameter(DIMENSION_SIZE);
			int width        = atoi(dsPar->getValue(0).c_str());
			int height       = atoi(dsPar->getValue(1).c_str());
			int nSlices      = atoi(dsPar->getValue(2).c_str());
			int nRepetitions = atoi(dsPar->getValue(3).c_str());
		
			Logger::getInstance().debug("[VistaWriter] getting PATIENT_NAME parameter");
			ImageDatasetParameter* patientNameParameter             = (*dataset)->getParameter(PATIENT_NAME);
			Logger::getInstance().debug("[VistaWriter] getting PATIENT_BIRTHDAY parameter");
			ImageDatasetParameter* patientBirthdayParameter         = (*dataset)->getParameter(PATIENT_BIRTHDAY);
			Logger::getInstance().debug("[VistaWriter] getting PATIENT_SEX parameter");
			ImageDatasetParameter* patientSexParameter              = (*dataset)->getParameter(PATIENT_SEX);
			Logger::getInstance().debug("[VistaWriter] getting MODALITY parameter");
			ImageDatasetParameter* modalityParameter                = (*dataset)->getParameter(MODALITY);
			Logger::getInstance().debug("[VistaWriter] getting DEVICE parameter");
			ImageDatasetParameter* deviceParameter                  = (*dataset)->getParameter(DEVICE);
			Logger::getInstance().debug("[VistaWriter] getting DEVICE_SOFTWARE parameter");
			ImageDatasetParameter* deviceSoftwareParameter          = (*dataset)->getParameter(DEVICE_SOFTWARE);
			Logger::getInstance().debug("[VistaWriter] getting COIL_ID parameter");
			ImageDatasetParameter* coilIDParameter                  = (*dataset)->getParameter(COIL_ID);
			Logger::getInstance().debug("[VistaWriter] getting METHOD_NAME parameter");
			ImageDatasetParameter* methodNameParameter              = (*dataset)->getParameter(METHOD_NAME);
			Logger::getInstance().debug("[VistaWriter] getting PROTOCOL_NAME parameter");
			ImageDatasetParameter* protocolNameParameter            = (*dataset)->getParameter(PROTOCOL_NAME);
			Logger::getInstance().debug("[VistaWriter] getting AQUISITION_DATE parameter");
			ImageDatasetParameter* dateParameter                    = (*dataset)->getParameter(AQUISITION_DATE);
			Logger::getInstance().debug("[VistaWriter] getting AQUISITION_TIME parameter");
			ImageDatasetParameter* timeParameter                    = (*dataset)->getParameter(AQUISITION_TIME);
			Logger::getInstance().debug("[VistaWriter] getting ECHO_TIME parameter");
			ImageDatasetParameter* echoTimeParameter                = (*dataset)->getParameter(ECHO_TIME);
			Logger::getInstance().debug("[VistaWriter] getting FIELD_OF_VIEW parameter");
			ImageDatasetParameter* fovParameter                     = (*dataset)->getParameter(FIELD_OF_VIEW);
			Logger::getInstance().debug("[VistaWriter] getting VOXEL_SIZE parameter");
			ImageDatasetParameter* voxelSizeParameter               = (*dataset)->getParameter(VOXEL_SIZE);
			Logger::getInstance().debug("[VistaWriter] getting REPETITION_TIME parameter");
			ImageDatasetParameter* repetitionTimeParameter          = (*dataset)->getParameter(REPETITION_TIME);
			Logger::getInstance().debug("[VistaWriter] getting FLIP_ANGLE parameter");
			ImageDatasetParameter* flipAngleParameter               = (*dataset)->getParameter(FLIP_ANGLE);
			Logger::getInstance().debug("[VistaWriter] getting SLICE_PACK_NUMBER_OF_SLICES parameter");
			ImageDatasetParameter* slicePackNumberOfSlicesParameter = (*dataset)->getParameter(SLICE_PACK_NUMBER_OF_SLICES);
			Logger::getInstance().debug("[VistaWriter] getting SLICE_PACK_AXES parameter");
			ImageDatasetParameter* slicePackAxesParameter           = (*dataset)->getParameter(SLICE_PACK_AXES);
			Logger::getInstance().debug("[VistaWriter] getting SLICE_PACK_POSITION parameter");
			ImageDatasetParameter* slicePackPositionParameter       = (*dataset)->getParameter(SLICE_PACK_POSITION);
			Logger::getInstance().debug("[VistaWriter] getting SLICE_PACK_SLICE_DISTANCE parameter");
			ImageDatasetParameter* slicePackSliceDistanceParameter  = (*dataset)->getParameter(SLICE_PACK_SLICE_DISTANCE);
			Logger::getInstance().debug("[VistaWriter] getting SLICE_PACK_SLICE_GAP parameter");
//			ImageDatasetParameter* slicePackSliceGapParameter       = (*dataset)->getParameter(SLICE_PACK_SLICE_GAP);
			Logger::getInstance().debug("[VistaWriter] getting SLICE_PACK_ORIENTATION parameter");
			ImageDatasetParameter* slicePackOrientationParameter    = (*dataset)->getParameter(SLICE_PACK_ORIENTATION);
			Logger::getInstance().debug("[VistaWriter] getting TMP_NUMBER_OF_ECHOES parameter");
			ImageDatasetParameter* tmpNumberOfEchoesParameter    = (*dataset)->getParameter(TMP_NUMBER_OF_ECHOES);
			Logger::getInstance().debug("[VistaWriter] getting TMP_ECHO_TIME_ARRAY parameter");
			ImageDatasetParameter* tmpEchoTimeArrayParameter    = (*dataset)->getParameter(TMP_ECHO_TIME_ARRAY);


//			Logger::getInstance().debug("[VistaWriter] tmpNumberOfEchoesParameter: " + tmpNumberOfEchoesParameter->getValue());

			ImageDatasetParameter* dataTypeParameter = (*dataset)->getParameter(DATA_TYPE);
			bool sourceIsFloat = dataTypeParameter->getValue().compare("source.format.float") == 0 ? true : false;
			Logger::getInstance().debug("[VistaWriter] source format: " + dataTypeParameter->getValue());

			
			int nEchoes;
			if(!tmpNumberOfEchoesParameter || sourceIsFloat) {
				nEchoes = 1;
			} else {
				nEchoes = atoi(tmpNumberOfEchoesParameter->getValue().c_str());
			}

			Logger::getInstance().debug("[VistaWriter] nEchoes:      " + nEchoes);
			Logger::getInstance().debug("[VistaWriter] nRepetitions: " + nRepetitions);
			Logger::getInstance().debug("[VistaWriter] nSlices:      " + nSlices);

			// for the time being the outmost loop is the loop over all echo times
			for(int _echo = 0; _echo < nEchoes; _echo++) {
				// we loop over	all repetitions and create one Vista image per repetition
				for(int _repetition = 0; _repetition < nRepetitions; _repetition++) {
	
					VImage _image;
					
					if(sourceIsFloat) {
						_image = VCreateImage(nSlices, height, width, VFloatRepn);
					} else {
						_image = VCreateImage(nSlices, height, width, VShortRepn);
					}
					
					if(slicePackOrientationParameter->getValue().compare("axial") == 0 ||
						slicePackOrientationParameter->getValue().compare("coronal") == 0 ||
						slicePackOrientationParameter->getValue().compare("transversal") == 0) {
						// need to flip the images (and change 'transversal' to 'axial')
						if(slicePackOrientationParameter->getValue().compare("transversal") == 0) {
							slicePackOrientationParameter = new ImageDatasetParameter(slicePackOrientationParameter->getName(), "axial");
						}
						for(int s = 0; s < nSlices; s++) {
							Image* sliceImage = (*dataset)->getImage(_echo + (s * nEchoes) + (_repetition * nSlices));
							for(int y = 0; y < height; y++) {
								for(int x = 0; x < width; x++) {
									if(sourceIsFloat) {
										VSetPixel(_image, s, y, width - x - 1, (float)sliceImage->getPixelValue(x, y));
									} else {
										VSetPixel(_image, s, y, width - x - 1, (short)sliceImage->getPixelValue(x, y));
									}
								}
							}
						}
					} else {
						for(int s = 0; s < nSlices; s++) {
							Image* sliceImage = (*dataset)->getImage(_echo + (s * nEchoes) + (_repetition * nSlices));
							for(int y = 0; y < height; y++) {
								for(int x = 0; x < width; x++) {
									if(sourceIsFloat) {
										VSetPixel(_image, s, y, x, (float)sliceImage->getPixelValue(x, y));
									} else {
										VSetPixel(_image, s, y, x, (short)sliceImage->getPixelValue(x, y));
									}
								}
							}
						}
					}
		
				
					string outputPrecision;
					configuration->getConfigurationValue("output.precision", outputPrecision);
		
					// if precision is auto or byte we have to 'scale' the images
					if(!sourceIsFloat && (outputPrecision.compare("auto") == 0 || outputPrecision.compare("byte") == 0)) {
						Logger::getInstance().debug("scaling intensity ...");
						// TODO: we should implement some convenience methods for getting
						//       double, int, bool, ... configuration values
						string cutOffLower, cutOffUpper;
						configuration->getConfigurationValue("histogram.cut.off.lower", cutOffLower);
						configuration->getConfigurationValue("histogram.cut.off.upper", cutOffUpper);
						_image = scaleSliceIntensity(_image, (double)atof(cutOffUpper.c_str()), (double)atof(cutOffLower.c_str()));
		//				LinearContrast<VShort> (_image, 0, 255, 0.01, 0.01);
		//				ConvertType<VShort,VUByte> (_image, VUByteRepn);
						 
						Logger::getInstance().debug("... scaling intensity finished");
					}
					// append our parameters
					VAttrList _aList = VImageAttrList(_image);
		
					VAppendAttr(_aList, "bandtype", NULL, VStringRepn, "spatial");
					Logger::getInstance().debug("[VistaWriter] appending 'patient' attribute");
					string _pn = (patientNameParameter->getValue().size() == 0 ? " " : patientNameParameter->getValue());
					VAppendAttr(_aList, "patient", NULL, VStringRepn, _pn.c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'birth' attribute");
					string _pb = (patientBirthdayParameter->getValue().size() == 0 ? " " : patientBirthdayParameter->getValue());
					VAppendAttr(_aList, "birth", NULL, VStringRepn, _pb.c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'sex' attribute");
					string _ps = (patientSexParameter->getValue().size() == 0 ? " " : patientSexParameter->getValue());
					VAppendAttr(_aList, "sex", NULL, VStringRepn, _ps.c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'modality' attribute");
					VAppendAttr(_aList, "modality", NULL, VStringRepn, modalityParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'device' attribute");
					VAppendAttr(_aList, "device", NULL, VStringRepn, deviceParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'device_software' attribute");
					VAppendAttr(_aList, "device_software", NULL, VStringRepn, deviceSoftwareParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'coil_id' attribute");
					VAppendAttr(_aList, "coil_id", NULL, VStringRepn, coilIDParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'method' attribute");
					VAppendAttr(_aList, "method", NULL, VStringRepn, methodNameParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'protocol' attribute");
					VAppendAttr(_aList, "protocol", NULL, VStringRepn, protocolNameParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'date' attribute");
					VAppendAttr(_aList, "date", NULL, VStringRepn, dateParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'time' attribute");
					VAppendAttr(_aList, "time", NULL, VStringRepn, timeParameter->getValue().c_str());
		
					Logger::getInstance().debug("[VistaWriter] appending 'x-axis' attribute");
					stringstream _xass;
					_xass << slicePackAxesParameter->getValue(0) << " " << slicePackAxesParameter->getValue(3) << " " << slicePackAxesParameter->getValue(6);
					VAppendAttr(_aList, "x-axis", NULL, VStringRepn, _xass.str().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'y-axis' attribute");
					stringstream _yass;
					_yass << slicePackAxesParameter->getValue(1) << " " << slicePackAxesParameter->getValue(4) << " " << slicePackAxesParameter->getValue(7);
					VAppendAttr(_aList, "y-axis", NULL, VStringRepn, _yass.str().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'z-axis' attribute");
					stringstream _zass;
					_zass << slicePackAxesParameter->getValue(2) << " " << slicePackAxesParameter->getValue(5) << " " << slicePackAxesParameter->getValue(8);
					VAppendAttr(_aList, "z-axis", NULL, VStringRepn, _zass.str().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'convention' attribute");
					VAppendAttr(_aList, "convention", NULL, VStringRepn, "natural");
		
					// location
					Logger::getInstance().debug("[VistaWriter] appending 'location' attribute");
					float _pos     = atof(slicePackPositionParameter->getValue(0).c_str());
					float _nSlices = atof(slicePackNumberOfSlicesParameter->getValue(0).c_str());
					float _dist    = atof(slicePackSliceDistanceParameter->getValue(0).c_str());
		
					float _location = _pos - ((_nSlices - 1) * _dist * 0.5);
					VAppendAttr(_aList, "location", NULL, VFloatRepn, _location);
		
					// in the 3D case we've got three values
					Logger::getInstance().debug("[VistaWriter] appending 'fov' attribute");
					if(fovParameter->getValueNumber() == 3) {
						string _fovString = fovParameter->getValue(0) + " " + fovParameter->getValue(1) + " " + fovParameter->getValue(2);
						VAppendAttr(_aList, "fov", NULL, VStringRepn, _fovString.c_str());
					} else {
						string _fovString = fovParameter->getValue(0) + " " + fovParameter->getValue(1);
						VAppendAttr(_aList, "fov", NULL, VStringRepn, _fovString.c_str());
					}
					Logger::getInstance().debug("[VistaWriter] appending 'voxel' attribute");
					string _voxelSizeString = voxelSizeParameter->getValue(0) + " " + voxelSizeParameter->getValue(1) + " " + voxelSizeParameter->getValue(2);
					VAppendAttr(_aList, "voxel", NULL, VStringRepn, _voxelSizeString.c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'orientation' attribute");
					VAppendAttr(_aList, "orientation", NULL, VStringRepn, slicePackOrientationParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'flip_angle' attribute");
					VAppendAttr(_aList, "flip_angle", NULL, VStringRepn, flipAngleParameter->getValue().c_str());
					Logger::getInstance().debug("[VistaWriter] appending 'echo_time' attribute");
					if(tmpEchoTimeArrayParameter) {
						VAppendAttr(_aList, "echo_time", NULL, VStringRepn, tmpEchoTimeArrayParameter->getValue(_echo).c_str());
					}
					Logger::getInstance().debug("[VistaWriter] appending 'repetition_time' attribute");
					VAppendAttr(_aList, "repetition_time", NULL, VStringRepn, repetitionTimeParameter->getValue().c_str());
		
		
					convertedImages.push_back(_image);
				} // end of repetition loop
			} // end of echo loop
		} else if (imageType.compare("DTI") == 0) {

			Logger::getInstance().info("dataset type is DTI");

			ImageDatasetParameter* dsPar;
			
			dsPar = (*dataset)->getParameter(DIMENSION_SIZE);
			int width        = atoi(dsPar->getValue(0).c_str());
			int height       = atoi(dsPar->getValue(1).c_str());
			int nSlices      = atoi(dsPar->getValue(2).c_str());
			int nDirections  = atoi(dsPar->getValue(3).c_str()); // it's not really the number of directions (beware of the A0 images)
																 // it's actually the number of images per slice which is the number
																 // of A0 images plus the number of directions

			// TODO: create a private method for doing the parameter nonsense to make
			//       make sure functional and anatomical datasets are 'in sync'
			ImageDatasetParameter* patientNameParameter                      = (*dataset)->getParameter(PATIENT_NAME);
			ImageDatasetParameter* patientBirthdayParameter                  = (*dataset)->getParameter(PATIENT_BIRTHDAY);
			ImageDatasetParameter* patientSexParameter                       = (*dataset)->getParameter(PATIENT_SEX);
			ImageDatasetParameter* modalityParameter                         = (*dataset)->getParameter(MODALITY);
			ImageDatasetParameter* deviceParameter                           = (*dataset)->getParameter(DEVICE);
			ImageDatasetParameter* deviceSoftwareParameter                   = (*dataset)->getParameter(DEVICE_SOFTWARE);
			ImageDatasetParameter* coilIDParameter                           = (*dataset)->getParameter(COIL_ID);
			ImageDatasetParameter* methodNameParameter                       = (*dataset)->getParameter(METHOD_NAME);
			ImageDatasetParameter* protocolNameParameter                     = (*dataset)->getParameter(PROTOCOL_NAME);
			ImageDatasetParameter* dateParameter                             = (*dataset)->getParameter(AQUISITION_DATE);
			ImageDatasetParameter* timeParameter                             = (*dataset)->getParameter(AQUISITION_TIME);
			ImageDatasetParameter* echoTimeParameter                         = (*dataset)->getParameter(ECHO_TIME);
			ImageDatasetParameter* fovParameter                              = (*dataset)->getParameter(FIELD_OF_VIEW);
			ImageDatasetParameter* voxelSizeParameter                        = (*dataset)->getParameter(VOXEL_SIZE);
			ImageDatasetParameter* repetitionTimeParameter                   = (*dataset)->getParameter(REPETITION_TIME);
			ImageDatasetParameter* flipAngleParameter                        = (*dataset)->getParameter(FLIP_ANGLE);
			ImageDatasetParameter* slicePackNumberOfSlicesParameter          = (*dataset)->getParameter(SLICE_PACK_NUMBER_OF_SLICES);
			ImageDatasetParameter* slicePackAxesParameter                    = (*dataset)->getParameter(SLICE_PACK_AXES);
			ImageDatasetParameter* slicePackPositionParameter                = (*dataset)->getParameter(SLICE_PACK_POSITION);
			ImageDatasetParameter* slicePackSliceDistanceParameter           = (*dataset)->getParameter(SLICE_PACK_SLICE_DISTANCE);
//			ImageDatasetParameter* slicePackSliceGapParameter                = (*dataset)->getParameter(SLICE_PACK_SLICE_GAP);
			ImageDatasetParameter* slicePackOrientationParameter             = (*dataset)->getParameter(SLICE_PACK_ORIENTATION);
			ImageDatasetParameter* numberOfDiffusionDirectionsParameter      = (*dataset)->getParameter(NUMBER_OF_DIFFUSION_DIRECTIONS);
			ImageDatasetParameter* numberOfExperimentsEachDirectionParameter = (*dataset)->getParameter(NUMBER_OF_EXPERIMENTS_IN_EACH_DIRECTION);
			ImageDatasetParameter* numberOfA0ImagesParameter                 = (*dataset)->getParameter(NUMBER_OF_A0_IMAGES);
			ImageDatasetParameter* diffusionDirectionsParameter              = (*dataset)->getParameter(DIFFUSION_DIRECTIONS);
			ImageDatasetParameter* diffusionEffectiveBValueParameter         = (*dataset)->getParameter(DIFFUSION_EFFECTIVE_B_VALUES);
			ImageDatasetParameter* numberOfRepetitionsParameter              = (*dataset)->getParameter(NUMBER_OF_REPETITIONS);

			int nA0Images = atoi(numberOfA0ImagesParameter->getValue().c_str());
			int nDiffDirs = atoi(numberOfDiffusionDirectionsParameter->getValue().c_str());	
			
			int nExpEachDir    = atoi(numberOfExperimentsEachDirectionParameter->getValue().c_str());
			int nOfRepetitions = atoi(numberOfRepetitionsParameter->getValue().c_str());
			

			for(int _repetition = 0; _repetition < nOfRepetitions; _repetition++) {
				for(int a0Image = 0; a0Image < nA0Images; a0Image++) {
					VImage _image = VCreateImage(nSlices, height, width, VShortRepn);
					stringstream _msg;
					_msg << "converting A0 image " << (a0Image + 1);
					Logger::getInstance().info(_msg.str());
					if(slicePackOrientationParameter->getValue().compare("axial") == 0 ||
						slicePackOrientationParameter->getValue().compare("coronal") == 0 ||
						slicePackOrientationParameter->getValue().compare("transversal") == 0) {
						// need to flip the images (and change 'transversal' to 'axial'
						if(slicePackOrientationParameter->getValue().compare("transversal") == 0) {
							slicePackOrientationParameter = new ImageDatasetParameter(slicePackOrientationParameter->getName(), "axial");
						}
						for(int slice = 0; slice < nSlices; slice++) {
							Image* dirImage = (*dataset)->getImage(_repetition * (nA0Images + (nDiffDirs * nExpEachDir)) * nSlices + slice + (a0Image * nSlices));
							for(int y = 0; y < height; y++) {
								for(int x = 0; x < width; x++) {
									VSetPixel(_image, slice, y, width - x - 1, (short)dirImage->getPixelValue(x, y));
								}
							}
						}
					} else {
						for(int slice = 0; slice < nSlices; slice++) {
							Image* dirImage = (*dataset)->getImage(_repetition * (nA0Images + (nDiffDirs * nExpEachDir)) * nSlices + slice + (a0Image * nSlices));
							for(int y = 0; y < height; y++) {
								for(int x = 0; x < width; x++) {
									VSetPixel(_image, slice, y, x, (short)dirImage->getPixelValue(x, y));
								}
							}
						}
					}
					
					// append our parameters
					VAttrList _aList = VImageAttrList(_image);
	
					VAppendAttr(_aList, "bandtype", NULL, VStringRepn, "spatial");
					string _pn = (patientNameParameter->getValue().size() == 0 ? " " : patientNameParameter->getValue());
					VAppendAttr(_aList, "patient", NULL, VStringRepn, _pn.c_str());
					string _pb = (patientBirthdayParameter->getValue().size() == 0 ? " " : patientBirthdayParameter->getValue());
					VAppendAttr(_aList, "birth", NULL, VStringRepn, _pb.c_str());
					string _ps = (patientSexParameter->getValue().size() == 0 ? " " : patientSexParameter->getValue());
					VAppendAttr(_aList, "sex", NULL, VStringRepn, _ps.c_str());
					VAppendAttr(_aList, "modality", NULL, VStringRepn, modalityParameter->getValue().c_str());
					VAppendAttr(_aList, "device", NULL, VStringRepn, deviceParameter->getValue().c_str());
					VAppendAttr(_aList, "device_software", NULL, VStringRepn, deviceSoftwareParameter->getValue().c_str());
					VAppendAttr(_aList, "coil_id", NULL, VStringRepn, coilIDParameter->getValue().c_str());
					VAppendAttr(_aList, "method", NULL, VStringRepn, methodNameParameter->getValue().c_str());
					VAppendAttr(_aList, "protocol", NULL, VStringRepn, protocolNameParameter->getValue().c_str());
					VAppendAttr(_aList, "date", NULL, VStringRepn, dateParameter->getValue().c_str());
					VAppendAttr(_aList, "time", NULL, VStringRepn, timeParameter->getValue().c_str());
	
					// axes				
					stringstream _xass;
					_xass << slicePackAxesParameter->getValue(0) << " " << slicePackAxesParameter->getValue(3) << " " << slicePackAxesParameter->getValue(6);
					VAppendAttr(_aList, "x-axis", NULL, VStringRepn, _xass.str().c_str());
					stringstream _yass;
					_yass << slicePackAxesParameter->getValue(1) << " " << slicePackAxesParameter->getValue(4) << " " << slicePackAxesParameter->getValue(7);
					VAppendAttr(_aList, "y-axis", NULL, VStringRepn, _yass.str().c_str());
					stringstream _zass;
					_zass << slicePackAxesParameter->getValue(2) << " " << slicePackAxesParameter->getValue(5) << " " << slicePackAxesParameter->getValue(8);
					VAppendAttr(_aList, "z-axis", NULL, VStringRepn, _zass.str().c_str());
					VAppendAttr(_aList, "convention", NULL, VStringRepn, "natural");
					
					// location
					float _pos     = atof(slicePackPositionParameter->getValue(0).c_str());
					float _nSlices = atof(slicePackNumberOfSlicesParameter->getValue(0).c_str());
					float _dist    = atof(slicePackSliceDistanceParameter->getValue(0).c_str());
		
					float _location = _pos - ((_nSlices - 1) * _dist * 0.5);
					VAppendAttr(_aList, "location", NULL, VFloatRepn, _location);
	
					// diffusion directions and friends
					VAppendAttr(_aList, "number_of_a0_images", NULL, VStringRepn, numberOfA0ImagesParameter->getValue().c_str());
					VAppendAttr(_aList, "number_of_diffusion_directions", NULL, VStringRepn, numberOfDiffusionDirectionsParameter->getValue().c_str());
					VAppendAttr(_aList, "diffusionDirections", NULL, VStringRepn, numberOfDiffusionDirectionsParameter->getValue().c_str());
					stringstream _ddss;
					// this is for A0 images
					// TODO: need to ask someone more competent if this is an OK thing to do
					_ddss << "0 0 0";
					VAppendAttr(_aList, "diffusionGradientOrientation", NULL, VStringRepn, _ddss.str().c_str());
					VAppendAttr(_aList, "diffusionBValue", NULL, VStringRepn, diffusionEffectiveBValueParameter->getValue(a0Image).c_str());
					
					string _fovString = fovParameter->getValue(0) + " " + fovParameter->getValue(1);
					VAppendAttr(_aList, "fov", NULL, VStringRepn, _fovString.c_str());
					string _voxelSizeString = voxelSizeParameter->getValue(0) + " " + voxelSizeParameter->getValue(1) + " " + voxelSizeParameter->getValue(2);
					VAppendAttr(_aList, "voxel", NULL, VStringRepn, _voxelSizeString.c_str());
					VAppendAttr(_aList, "orientation", NULL, VStringRepn, slicePackOrientationParameter->getValue().c_str());
					VAppendAttr(_aList, "flip_angle", NULL, VStringRepn, flipAngleParameter->getValue().c_str());
					VAppendAttr(_aList, "echo_time", NULL, VStringRepn, echoTimeParameter->getValue().c_str());
					VAppendAttr(_aList, "repetition_time", NULL, VStringRepn, repetitionTimeParameter->getValue().c_str());
					
					
					convertedImages.push_back(_image);
				}
	
				for(int direction = 0; direction < nDiffDirs; direction++) {
					stringstream _msg;
					_msg << "converting diffusion direction " << (direction + 1);
					Logger::getInstance().info(_msg.str());
					for(int exp = 0; exp < nExpEachDir; exp++) {
						VImage _image = VCreateImage(nSlices, height, width, VShortRepn);
						stringstream _msg;
						_msg << "converting all directions for B value number " << (exp + 1);
						Logger::getInstance().info(_msg.str());
						if(slicePackOrientationParameter->getValue().compare("axial") == 0 ||
							slicePackOrientationParameter->getValue().compare("coronal") == 0 ||
							slicePackOrientationParameter->getValue().compare("transversal") == 0) {
							// need to flip the images (and change 'transversal' to 'axial'
							if(slicePackOrientationParameter->getValue().compare("transversal") == 0) {
								slicePackOrientationParameter = new ImageDatasetParameter(slicePackOrientationParameter->getName(), "axial");
							}
							for(int slice = 0; slice < nSlices; slice++) {
								Image* dirImage = (*dataset)->getImage(_repetition * (nA0Images + (nDiffDirs * nExpEachDir)) * nSlices + (nA0Images + (direction * nExpEachDir) + exp) * nSlices + slice);
								for(int y = 0; y < height; y++) {
									for(int x = 0; x < width; x++) {
										VSetPixel(_image, slice, y, width - x - 1, (short)dirImage->getPixelValue(x, y));
									}
								}
							}
						} else {
							for(int slice = 0; slice < nSlices; slice++) {
								Image* dirImage = (*dataset)->getImage(_repetition * (nA0Images + (nDiffDirs * nExpEachDir)) * nSlices + (nA0Images + (direction * nExpEachDir) + exp) * nSlices + slice);
								for(int y = 0; y < height; y++) {
									for(int x = 0; x < width; x++) {
										VSetPixel(_image, slice, y, x, (short)dirImage->getPixelValue(x, y));
									}
								}
							}
						}
						
						// append our parameters
						VAttrList _aList = VImageAttrList(_image);
		
						string _pn = (patientNameParameter->getValue().size() == 0 ? " " : patientNameParameter->getValue());
						VAppendAttr(_aList, "patient", NULL, VStringRepn, _pn.c_str());
						string _pb = (patientBirthdayParameter->getValue().size() == 0 ? " " : patientBirthdayParameter->getValue());
						VAppendAttr(_aList, "birth", NULL, VStringRepn, _pb.c_str());
						string _ps = (patientSexParameter->getValue().size() == 0 ? " " : patientSexParameter->getValue());
						VAppendAttr(_aList, "sex", NULL, VStringRepn, _ps.c_str());
						VAppendAttr(_aList, "modality", NULL, VStringRepn, modalityParameter->getValue().c_str());
						VAppendAttr(_aList, "device", NULL, VStringRepn, deviceParameter->getValue().c_str());
						VAppendAttr(_aList, "device_software", NULL, VStringRepn, deviceSoftwareParameter->getValue().c_str());
						VAppendAttr(_aList, "coil_id", NULL, VStringRepn, coilIDParameter->getValue().c_str());
						VAppendAttr(_aList, "method", NULL, VStringRepn, methodNameParameter->getValue().c_str());
						VAppendAttr(_aList, "protocol", NULL, VStringRepn, protocolNameParameter->getValue().c_str());
						VAppendAttr(_aList, "date", NULL, VStringRepn, dateParameter->getValue().c_str());
						VAppendAttr(_aList, "time", NULL, VStringRepn, timeParameter->getValue().c_str());
		
						// axes				
						stringstream _xass;
						_xass << slicePackAxesParameter->getValue(0) << " " << slicePackAxesParameter->getValue(3) << " " << slicePackAxesParameter->getValue(6);
						VAppendAttr(_aList, "x-axis", NULL, VStringRepn, _xass.str().c_str());
						stringstream _yass;
						_yass << slicePackAxesParameter->getValue(1) << " " << slicePackAxesParameter->getValue(4) << " " << slicePackAxesParameter->getValue(7);
						VAppendAttr(_aList, "y-axis", NULL, VStringRepn, _yass.str().c_str());
						stringstream _zass;
						_zass << slicePackAxesParameter->getValue(2) << " " << slicePackAxesParameter->getValue(5) << " " << slicePackAxesParameter->getValue(8);
						VAppendAttr(_aList, "z-axis", NULL, VStringRepn, _zass.str().c_str());
						VAppendAttr(_aList, "convention", NULL, VStringRepn, "natural");
						
						// location
						float _pos     = atof(slicePackPositionParameter->getValue(0).c_str());
						float _nSlices = atof(slicePackNumberOfSlicesParameter->getValue(0).c_str());
						float _dist    = atof(slicePackSliceDistanceParameter->getValue(0).c_str());
			
						float _location = _pos - ((_nSlices - 1) * _dist * 0.5);
						VAppendAttr(_aList, "location", NULL, VFloatRepn, _location);
		
						// diffusion directions and friends
						VAppendAttr(_aList, "number_of_a0_images", NULL, VStringRepn, numberOfA0ImagesParameter->getValue().c_str());
						VAppendAttr(_aList, "number_of_diffusion_directions", NULL, VStringRepn, numberOfDiffusionDirectionsParameter->getValue().c_str());
						VAppendAttr(_aList, "diffusionDirections", NULL, VStringRepn, numberOfDiffusionDirectionsParameter->getValue().c_str());
						stringstream _ddss;
						_ddss << diffusionDirectionsParameter->getValue(direction * 3) << " " << diffusionDirectionsParameter->getValue(direction * 3 + 1) << " " << diffusionDirectionsParameter->getValue(direction * 3 + 2);
						VAppendAttr(_aList, "diffusionGradientOrientation", NULL, VStringRepn, _ddss.str().c_str());
						VAppendAttr(_aList, "diffusionBValue", NULL, VStringRepn, diffusionEffectiveBValueParameter->getValue(nA0Images + (direction * nExpEachDir) + exp).c_str());
						
						string _fovString = fovParameter->getValue(0) + " " + fovParameter->getValue(1);
						VAppendAttr(_aList, "fov", NULL, VStringRepn, _fovString.c_str());
						string _voxelSizeString = voxelSizeParameter->getValue(0) + " " + voxelSizeParameter->getValue(1) + " " + voxelSizeParameter->getValue(2);
						VAppendAttr(_aList, "voxel", NULL, VStringRepn, _voxelSizeString.c_str());
						VAppendAttr(_aList, "orientation", NULL, VStringRepn, slicePackOrientationParameter->getValue().c_str());
						VAppendAttr(_aList, "flip_angle", NULL, VStringRepn, flipAngleParameter->getValue().c_str());
						VAppendAttr(_aList, "echo_time", NULL, VStringRepn, echoTimeParameter->getValue().c_str());
						VAppendAttr(_aList, "repetition_time", NULL, VStringRepn, repetitionTimeParameter->getValue().c_str());
						
						
						convertedImages.push_back(_image);
					}
				}
			}
		}
		++dataset;
	}
	

	VImage* imageArray = new VImage[convertedImages.size()];
	for(int counter = 0; counter < (int)convertedImages.size(); counter++) {
		imageArray[counter] = convertedImages[counter];
	}

	FILE *fp;
	string targetFile;
	configuration->getConfigurationValue(ConverterConfiguration::TARGET_URL_KEY, targetFile);
	targetFile.erase(0, targetFile.find_first_of(':') + 1);
    fp = VOpenOutputFile(targetFile.c_str(), TRUE);
    VWriteImages(fp, 0, convertedImages.size(), imageArray);
    fclose(fp);
	
}
/*
 * TODO: this is taken directly from the old brutov and needs
 *       a bit of cleaning and optimizing
 */
 
VImage VistaWriter::scaleSliceIntensity(VImage src, double white, double black, int fold) {
  /* scale any input image to VUByte,
     mapping white (black) percent of the voxel to white (black) */

  int x, y, z, zi, nx, ny, nz, i, range, maxshort;
  unsigned int lb, ub, limit, sum, *histo;
  double m, b, max, min, mean, var, v;
  VImage dst;

  maxshort = (int)(VRepnMaxValue(VShortRepn));
  histo = (unsigned int *)VCalloc(maxshort, sizeof(unsigned int));
  nx = VImageNColumns(src);
  ny = VImageNRows(src);
  nz = VImageNBands(src);

  if (white < 0 || white > 100 || black < 0 || black > 100 || white+black >= 100)  {
    fprintf(stderr, "VScaleIntensity: illegal percentage given.\n");
    return 0;
  };

  /* first pass: find maximum and minimum values */
  VImageStats(src, VAllBands, &min, &max, &mean, &var);
  if (max == min) {
    fprintf(stderr, "VScaleIntensity: illegal data in image.\n");
    return 0;
  };
  b = min;
  m = (max-min) / (double)maxshort;

  /* second pass: build a histogram*/
  for (z = 0; z < nz; z++)  {
    for (y = 0; y < ny; y++)  {
      for (x = 0; x < nx; x++)  {
	v = VGetPixel(src, z, y, x);
//	i = (int)((v-b)/m+0.5);
	i = (int)v;
	histo[i]++;
      };
    };
  };

  /* throw away pc percent of the voxel below lb and pc percent above ub */
  limit = int ((black * nx * ny * nz) / 100);
  lb = 0; sum = 0;
  for (i = 0; i < maxshort; i++)  {
    sum += histo[i];
    if (sum >= limit) { lb = i; break; };
  };
  limit = int ((white * nx * ny * nz) / 100);
  ub = maxshort-1; sum = 0;
  for (i = maxshort-1; i >= 0; i--)  {
    sum += histo[i];
    if (sum >= limit) { ub = i; break; };
  };
  min = lb;
  max = ub;

  /* third pass: create and convert image */
  dst = VCreateImage(nz, ny, nx, VUByteRepn);
  if (dst == 0) return 0;

  range = 256;
  m = range / (max - min);
  b = range - (m * max);
  for (z = 0; z < nz; z++)  {
    for (y = 0; y < ny; y++)  {
      for (x = 0; x < nx; x++)  {
	v = VGetPixel(src, z, y, x);
	i = (int)(v * m + b + 0.5);
	if (i < 0) i = 0;
	else if (i >= range) i = range-1;
	zi = z-fold; if (zi < 0) zi += nz;
	VSetPixel(dst, zi, y, x, i);
      };
    };
  };
  VFree(histo);
  return dst;
}



};
