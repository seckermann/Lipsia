#include "NiftiWriter.h"

/* global includes */
#include <iostream>
#include <fstream>
#include <ios>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>
#include <unistd.h>

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

namespace nifti
{
  
  /* according to the GNU libc library documentation, the following holds:
     "On 32bit systems, if _FILE_OFFSET_BITS == 64 the fopen function behaves 
     like the fopen64 implementation." 
     So, there is no need to do a special call to fopen64.
  */
#define _FILE_OFFSET_BITS 64

  ConverterConfiguration* configuration;

  NiftiWriter::NiftiWriter(ConverterConfiguration* cc) {
    configuration = cc;
  }

  NiftiWriter::~NiftiWriter() {
  }

  void NiftiWriter::write(vector<IImageDataset*> datasets) {

    Logger::getInstance().debug("[NiftiWriter] writing " + toString(datasets.size()) + " dataset(s)");


    vector<IImageDataset*>::iterator dataset = datasets.begin();
    int _dsCounter = 1;
    while(dataset != datasets.end()) {

      Logger::getInstance().info("[NiftiWriter] writing dataset " + toString(_dsCounter));

      string imageType = (*dataset)->getParameter(DATASET_TYPE)->getValue();

      if(imageType.compare("FUNCTIONAL") == 0) {

			Logger::getInstance().info("[NiftiWriter] dataset type is FUNCTIONAL");

			ImageDatasetParameter* dsPar = (*dataset)->getParameter(DIMENSION_SIZE);
			int width        = atoi(dsPar->getValue(0).c_str());
			int height       = atoi(dsPar->getValue(1).c_str());
			int nSlices      = atoi(dsPar->getValue(2).c_str());
			int nRepetitions = atoi(dsPar->getValue(3).c_str());
			int nEchoes      = 1;

			Logger::getInstance().debug("[NiftiWriter] getting VOXEL_SIZE parameter");
			ImageDatasetParameter* voxelSizeParameter               = (*dataset)->getParameter(VOXEL_SIZE);
			Logger::getInstance().debug("[NiftiWriter] getting REPETITION_TIME parameter");
			ImageDatasetParameter* repetitionTimeParameter          = (*dataset)->getParameter(REPETITION_TIME);
			Logger::getInstance().debug("[NiftiWriter] getting METHOD_NAME parameter");
			ImageDatasetParameter* methodNameParameter              = (*dataset)->getParameter(METHOD_NAME);

			//			Logger::getInstance().debug("[NiftiWriter] tmpNumberOfEchoesParameter: " + tmpNumberOfEchoesParameter->getValue());

			ImageDatasetParameter* dataTypeParameter = (*dataset)->getParameter(DATA_TYPE);
			bool sourceIsFloat = dataTypeParameter->getValue().compare("source.format.float") == 0 ? true : false;
			Logger::getInstance().debug("[NiftiWriter] source format: " + dataTypeParameter->getValue());


			Logger::getInstance().debug("[NiftiWriter] width:        " + toString(width));
			Logger::getInstance().debug("[NiftiWriter] height:       " + toString(height));
			Logger::getInstance().debug("[NiftiWriter] nSlices:      " + toString(nSlices));
			Logger::getInstance().debug("[NiftiWriter] nRepetitions: " + toString(nRepetitions));
			Logger::getInstance().debug("[NiftiWriter] nEchoes:      " + toString(nEchoes));

			nifti_1_header header;

			header.sizeof_hdr = 348;  // fixed size

			header.dim[0] = 4;  // number of dimensions
			header.dim[1] = width;
			header.dim[2] = height;
			header.dim[3] = nSlices;
			header.dim[4] = nRepetitions;
			header.dim[5] = 0;
			header.dim[6] = 0;
			header.dim[7] = 0;

			header.pixdim[1] = atof(voxelSizeParameter->getValue(0).c_str());
			header.pixdim[2] = atof(voxelSizeParameter->getValue(1).c_str());
			header.pixdim[3] = atof(voxelSizeParameter->getValue(2).c_str());
			header.pixdim[4] = atof(repetitionTimeParameter->getValue().c_str()) / 1000.0f;
			header.pixdim[5] = 0.0f;
			header.pixdim[6] = 0.0f;
			header.pixdim[7] = 0.0f;

			header.xyzt_units = NIFTI_UNITS_MM | NIFTI_UNITS_SEC;


			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[0]: " + toString(header.dim[0]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[1]: " + toString(header.dim[1]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[2]: " + toString(header.dim[2]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[3]: " + toString(header.dim[3]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[4]: " + toString(header.dim[4]));

			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[1]: " + toString(header.pixdim[1]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[2]: " + toString(header.pixdim[2]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[3]: " + toString(header.pixdim[3]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[4]: " + toString(header.pixdim[4]));

			writeOrientation(*dataset, header);

			// make spm happy
			header.slice_code     = 0;
			header.slice_duration = 0.0;
			header.slice_start    = 0;
			header.slice_end      = -1;

			header.cal_min = 0.0;
			header.cal_max = 0.0;

			header.glmin = 0;
			header.glmax = 0;

			header.scl_inter = 0.0;
			header.scl_slope = 0.0;

			header.toffset = 0.0;

			if(sourceIsFloat) {
			  header.datatype = DT_FLOAT32;
			  header.bitpix   = 32;
			} else {
			  header.datatype = DT_INT16;
			  header.bitpix   = 16;
			}
			header.vox_offset = 352;

			header.magic[0] = 'n';
			header.magic[1] = '+';
			header.magic[2] = '1';
			header.magic[3] = 0;

			strcpy(header.descrip, methodNameParameter->getValue().c_str());

			FILE *fp;
			string targetFile;
			configuration->getConfigurationValue(ConverterConfiguration::TARGET_URL_KEY, targetFile);
			// extract actual file name
			targetFile.erase(0, targetFile.find_first_of(':') + 1);

			fp = fopen(targetFile.c_str(), "wb+");

			// write header
			fwrite(&header, 348, 1, fp);
			// write extension header
			int _noExtensions = 0;
			fwrite(&_noExtensions, 4, 1, fp);

			short _lineBuffer[width];
			float _lineBufferFloat[width];
			for(int _repetition = 0; _repetition < nRepetitions; _repetition++) {
			  for(int s = 0; s < nSlices; s++) {
				Image* sliceImage = (*dataset)->getImage(s + (_repetition * nSlices));
				for(int y = 0; y < height; y++) {
				  if(sourceIsFloat) {
				for(int x = 0; x < width; x++) {
				  _lineBufferFloat[x] = (float)sliceImage->getPixelValue(x, y);
				}
				fwrite(_lineBufferFloat, 4, width, fp);
				  } else {
				for(int x = 0; x < width; x++) {
				  _lineBuffer[x] = (short)sliceImage->getPixelValue(x, y);
				}
				fwrite(_lineBuffer, 2, width, fp);
				  }
				}
			  }
			} // end of repetition loop



			fclose(fp);

      } else if (imageType.compare("ANATOMICAL") == 0) {
			// this is ANATOMICAL

			Logger::getInstance().info("[NiftiWriter] dataset type is ANATOMICAL");

			ImageDatasetParameter* dsPar = (*dataset)->getParameter(DIMENSION_SIZE);
			int width        = atoi(dsPar->getValue(0).c_str());
			int height       = atoi(dsPar->getValue(1).c_str());
			int nSlices      = atoi(dsPar->getValue(2).c_str());
			int nRepetitions = atoi(dsPar->getValue(3).c_str());

			Logger::getInstance().debug("[NiftiWriter] getting PATIENT_NAME parameter");
			ImageDatasetParameter* patientNameParameter             = (*dataset)->getParameter(PATIENT_NAME);
			Logger::getInstance().debug("[NiftiWriter] getting PATIENT_BIRTHDAY parameter");
			ImageDatasetParameter* patientBirthdayParameter         = (*dataset)->getParameter(PATIENT_BIRTHDAY);
			Logger::getInstance().debug("[NiftiWriter] getting PATIENT_SEX parameter");
			ImageDatasetParameter* patientSexParameter              = (*dataset)->getParameter(PATIENT_SEX);
			Logger::getInstance().debug("[NiftiWriter] getting MODALITY parameter");
			ImageDatasetParameter* modalityParameter                = (*dataset)->getParameter(MODALITY);
			Logger::getInstance().debug("[NiftiWriter] getting DEVICE parameter");
			ImageDatasetParameter* deviceParameter                  = (*dataset)->getParameter(DEVICE);
			Logger::getInstance().debug("[NiftiWriter] getting DEVICE_SOFTWARE parameter");
			ImageDatasetParameter* deviceSoftwareParameter          = (*dataset)->getParameter(DEVICE_SOFTWARE);
			Logger::getInstance().debug("[NiftiWriter] getting COIL_ID parameter");
			ImageDatasetParameter* coilIDParameter                  = (*dataset)->getParameter(COIL_ID);
			Logger::getInstance().debug("[NiftiWriter] getting METHOD_NAME parameter");
			ImageDatasetParameter* methodNameParameter              = (*dataset)->getParameter(METHOD_NAME);
			Logger::getInstance().debug("[NiftiWriter] getting PROTOCOL_NAME parameter");
			ImageDatasetParameter* protocolNameParameter            = (*dataset)->getParameter(PROTOCOL_NAME);
			Logger::getInstance().debug("[NiftiWriter] getting AQUISITION_DATE parameter");
			ImageDatasetParameter* dateParameter                    = (*dataset)->getParameter(AQUISITION_DATE);
			Logger::getInstance().debug("[NiftiWriter] getting AQUISITION_TIME parameter");
			ImageDatasetParameter* timeParameter                    = (*dataset)->getParameter(AQUISITION_TIME);
			Logger::getInstance().debug("[NiftiWriter] getting ECHO_TIME parameter");
			ImageDatasetParameter* echoTimeParameter                = (*dataset)->getParameter(ECHO_TIME);
			Logger::getInstance().debug("[NiftiWriter] getting FIELD_OF_VIEW parameter");
			ImageDatasetParameter* fovParameter                     = (*dataset)->getParameter(FIELD_OF_VIEW);
			Logger::getInstance().debug("[NiftiWriter] getting VOXEL_SIZE parameter");
			ImageDatasetParameter* voxelSizeParameter               = (*dataset)->getParameter(VOXEL_SIZE);
			Logger::getInstance().debug("[NiftiWriter] getting REPETITION_TIME parameter");
			ImageDatasetParameter* repetitionTimeParameter          = (*dataset)->getParameter(REPETITION_TIME);
			Logger::getInstance().debug("[NiftiWriter] getting FLIP_ANGLE parameter");
			ImageDatasetParameter* flipAngleParameter               = (*dataset)->getParameter(FLIP_ANGLE);
			Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_NUMBER_OF_SLICES parameter");
			ImageDatasetParameter* slicePackNumberOfSlicesParameter = (*dataset)->getParameter(SLICE_PACK_NUMBER_OF_SLICES);
			Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_ORIENTATION parameter");
			ImageDatasetParameter* slicePackOrientationParameter    = (*dataset)->getParameter(SLICE_PACK_ORIENTATION);
			Logger::getInstance().debug("[NiftiWriter] getting TMP_NUMBER_OF_ECHOES parameter");
			ImageDatasetParameter* tmpNumberOfEchoesParameter       = (*dataset)->getParameter(TMP_NUMBER_OF_ECHOES);
			Logger::getInstance().debug("[NiftiWriter] getting TMP_ECHO_TIME_ARRAY parameter");
			ImageDatasetParameter* tmpEchoTimeArrayParameter        = (*dataset)->getParameter(TMP_ECHO_TIME_ARRAY);


			//			Logger::getInstance().debug("[NiftiWriter] tmpNumberOfEchoesParameter: " + tmpNumberOfEchoesParameter->getValue());

			ImageDatasetParameter* dataTypeParameter = (*dataset)->getParameter(DATA_TYPE);
			bool sourceIsFloat = dataTypeParameter->getValue().compare("source.format.float") == 0 ? true : false;
			Logger::getInstance().debug("[NiftiWriter] source format: " + dataTypeParameter->getValue());


			int nEchoes;
			if(!tmpNumberOfEchoesParameter || sourceIsFloat) {
			  nEchoes = 1;
			} else {
			  nEchoes = atoi(tmpNumberOfEchoesParameter->getValue().c_str());
			}

			Logger::getInstance().debug("[NiftiWriter] width:        " + toString(width));
			Logger::getInstance().debug("[NiftiWriter] height:       " + toString(height));
			Logger::getInstance().debug("[NiftiWriter] nSlices:      " + toString(nSlices));
			Logger::getInstance().debug("[NiftiWriter] nRepetitions: " + toString(nRepetitions));
			Logger::getInstance().debug("[NiftiWriter] nEchoes:      " + toString(nEchoes));

			nifti_1_header header;

			header.sizeof_hdr = 348;  // fixed size

			if(nEchoes == 1) {
			  header.dim[0] = 3;  // number of dimensions
			  header.dim[1] = width;
			  header.dim[2] = height;
			  header.dim[3] = nSlices;
			  header.dim[4] = 0;
			  header.dim[5] = 0;
			  header.dim[6] = 0;
			  header.dim[7] = 0;
			} else {
			  header.dim[0] = 5;  // number of dimensions
			  header.dim[1] = width;
			  header.dim[2] = height;
			  header.dim[3] = nSlices;
			  header.dim[4] = 1; // nRepetitions should always be 1
			  header.dim[5] = nEchoes;
			  header.dim[6] = 0;
			  header.dim[7] = 0;
			}

			header.pixdim[1] = atof(voxelSizeParameter->getValue(0).c_str());
			header.pixdim[2] = atof(voxelSizeParameter->getValue(1).c_str());
			header.pixdim[3] = atof(voxelSizeParameter->getValue(2).c_str());
			header.pixdim[4] = 0.0f; // no repetitions
			if(nEchoes > 1) {
			  header.pixdim[5] = atof(tmpEchoTimeArrayParameter->getValue(1).c_str()) - atof(tmpEchoTimeArrayParameter->getValue(0).c_str());
			  header.intent_code = NIFTI_INTENT_DIMLESS;
			  header.intent_name[0] = 'E';
			  header.intent_name[1] = 'c';
			  header.intent_name[2] = 'h';
			  header.intent_name[3] = 'o';
			  header.intent_name[4] = 0;
			} else {
			  header.pixdim[5] = 0.0;
			}

			// make spm happy
			header.pixdim[6] = 0.0;
			header.pixdim[7] = 0.0;

			header.xyzt_units = NIFTI_UNITS_MM;

			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[0]: " + toString(header.dim[0]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[1]: " + toString(header.dim[1]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[2]: " + toString(header.dim[2]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[3]: " + toString(header.dim[3]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[4]: " + toString(header.dim[4]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.dim[5]: " + toString(header.dim[5]));

			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[1]: " + toString(header.pixdim[1]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[2]: " + toString(header.pixdim[2]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[3]: " + toString(header.pixdim[3]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[4]: " + toString(header.pixdim[4]));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[5]: " + toString(header.pixdim[5]));


			writeOrientation(*dataset, header);

			Logger::getInstance().debug("[NiftiWriter] nifti header.qform_code: " + toString(header.qform_code));
			Logger::getInstance().debug("[NiftiWriter] nifti header.quatern_b: " + toString(header.quatern_b));
			Logger::getInstance().debug("[NiftiWriter] nifti header.quatern_c: " + toString(header.quatern_c));
			Logger::getInstance().debug("[NiftiWriter] nifti header.quatern_d: " + toString(header.quatern_d));
			Logger::getInstance().debug("[NiftiWriter] nifti header.pixdim[0]: " + toString(header.pixdim[0]));

			Logger::getInstance().debug("[NiftiWriter] nifti header.qoffset_x: " + toString(header.qoffset_x));
			Logger::getInstance().debug("[NiftiWriter] nifti header.qoffset_y: " + toString(header.qoffset_y));
			Logger::getInstance().debug("[NiftiWriter] nifti header.qoffset_z: " + toString(header.qoffset_z));

			// make spm happy
			header.slice_code     = 0;
			header.slice_duration = 0.0;
			header.slice_start    = 0;
			header.slice_end      = -1;

			header.cal_min = 0.0;
			header.cal_max = 0.0;

			header.glmin = 0;
			header.glmax = 0;

			header.scl_inter = 0.0;
			header.scl_slope = 0.0;

			header.toffset = 0.0;

			if(sourceIsFloat) {
			  header.datatype = DT_FLOAT32;
			  header.bitpix   = 32;
			} else {
			  header.datatype = DT_INT16;
			  header.bitpix   = 16;
			}
			header.vox_offset = 352;

			header.magic[0] = 'n';
			header.magic[1] = '+';
			header.magic[2] = '1';
			header.magic[3] = 0;

			strcpy(header.descrip, methodNameParameter->getValue().c_str());

			FILE *fp;
			string targetFile;
			configuration->getConfigurationValue(ConverterConfiguration::TARGET_URL_KEY, targetFile);
			// extract actual file name
			targetFile.erase(0, targetFile.find_first_of(':') + 1);

			fp = fopen(targetFile.c_str(), "wb+");

			// write header
			fwrite(&header, 348, 1, fp);
			// write extension header
			int _noExtensions = 0;
			fwrite(&_noExtensions, 4, 1, fp);

			short _lineBuffer[width];
			float _lineBufferFloat[width];
			for(int _echo = 0; _echo < nEchoes; _echo++) {
			  for(int _repetition = 0; _repetition < nRepetitions; _repetition++) {
				for(int s = 0; s < nSlices; s++) {
				  Image* sliceImage = (*dataset)->getImage(_echo + (s * nEchoes) + (_repetition * nSlices));
				  for(int y = 0; y < height; y++) {
				if(sourceIsFloat) {
				  for(int x = 0; x < width; x++) {
					_lineBufferFloat[x] = (float)sliceImage->getPixelValue(x, height - y - 1);
				  }
				  fwrite(_lineBufferFloat, 4, width, fp);
				} else {
				  if(slicePackOrientationParameter->getValue().compare("sagittal") == 0) {
					for(int x = 0; x < width; x++) {
						  _lineBuffer[x] = (short)sliceImage->getPixelValue(x, height - y - 1);
					}
				  } else {
					for(int x = 0; x < width; x++) {
						  _lineBuffer[width - x - 1] = (short)sliceImage->getPixelValue(x, height - y - 1);
					}
				  }
				  fwrite(_lineBuffer, 2, width, fp);
				}
				  }
				}
			  } // end of repetition loop
			} // end of echo loop



			fclose(fp);

      } else if (imageType.compare("DTI") == 0) {

			Logger::getInstance().info("[NiftiWriter] dataset type is DTI");

			  }

			  ++dataset; // next dataset
			  ++_dsCounter;
			}



  }

  void NiftiWriter::writeOrientation(IImageDataset* dataset, nifti_1_header& nHeader) {

    string nno;
    configuration->getConfigurationValue("no.nifti.orientation", nno);
    bool _noNiftiOrientation = (nno.compare("yes") == 0);

    if(_noNiftiOrientation) {
      Logger::getInstance().debug("[NiftiWriter] 'no-nifti-orientation' option is set to yes");
      Logger::getInstance().debug("[NiftiWriter] not writing orientation parameters into Nifti header");
      nHeader.qform_code = 0;
      nHeader.quatern_b = 0.0f;
      nHeader.quatern_c = 0.0f;
      nHeader.quatern_d = 0.0f;
      nHeader.pixdim[0] = 0.0f;
      return;
    }

    Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_ORIENTATION parameter");
    ImageDatasetParameter* slicePackOrientationParameter     = dataset->getParameter(SLICE_PACK_ORIENTATION);
    Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_READ_ORIENTATION parameter");
    ImageDatasetParameter* slicePackReadOrientationParameter = dataset->getParameter(SLICE_PACK_READ_ORIENTATION);
    Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_AXES parameter");
    ImageDatasetParameter* slicePackAxesParameter            = dataset->getParameter(SLICE_PACK_AXES);
    Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_POSITION parameter");
    ImageDatasetParameter* slicePackPositionParameter        = dataset->getParameter(SLICE_PACK_POSITION);
    Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_SLICE_DISTANCE parameter");
    ImageDatasetParameter* slicePackSliceDistanceParameter   = dataset->getParameter(SLICE_PACK_SLICE_DISTANCE);
    Logger::getInstance().debug("[NiftiWriter] getting SLICE_PACK_OFFSET parameter");
    ImageDatasetParameter* slicePackOffsetParameter          = dataset->getParameter(SLICE_PACK_OFFSET);

    // get rotation matrix elements
    float R11 = atof(slicePackAxesParameter->getValue(0).c_str());
    float R21 = atof(slicePackAxesParameter->getValue(3).c_str());
    float R31 = atof(slicePackAxesParameter->getValue(6).c_str());
    float R12 = atof(slicePackAxesParameter->getValue(1).c_str());
    float R22 = atof(slicePackAxesParameter->getValue(4).c_str());
    float R32 = atof(slicePackAxesParameter->getValue(7).c_str());
    float R13 = atof(slicePackAxesParameter->getValue(2).c_str());
    float R23 = atof(slicePackAxesParameter->getValue(5).c_str());
    float R33 = atof(slicePackAxesParameter->getValue(8).c_str());

    float qfac = 1.0;

    Logger::getInstance().debug("[NiftiWriter] got Bruker Rotation Matrix:");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R11) + ", " + toString(R12) + ", " + toString(R13) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R21) + ", " + toString(R22) + ", " + toString(R23) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R31) + ", " + toString(R32) + ", " + toString(R33) + ")");

    float _readOffset  = atof(slicePackOffsetParameter->getValue(0).c_str());
    float _phaseOffset = atof(slicePackOffsetParameter->getValue(1).c_str());
    float _sliceOffset = atof(slicePackOffsetParameter->getValue(2).c_str());

    Logger::getInstance().debug("[NiftiWriter] got original Bruker slice pack offsets:");
    Logger::getInstance().debug("[NiftiWriter] read offset  = " + toString(_readOffset));
    Logger::getInstance().debug("[NiftiWriter] phase offset = " + toString(_phaseOffset));
    Logger::getInstance().debug("[NiftiWriter] slice offset = " + toString(_sliceOffset));

    float _det = (R11 * R22 * R33) + (R12 * R23 * R31) + (R13 * R21 * R32) - (R31 * R22 * R13) - (R32 * R23 * R11) - (R33 * R21 * R12);

    float iR11 = ((R22 * R33) - (R23 * R32)) / _det;
    float iR12 = ((R13 * R32) - (R12 * R33)) / _det;
    float iR13 = ((R12 * R23) - (R13 * R22)) / _det;
    float iR21 = ((R23 * R31) - (R21 * R33)) / _det;
    float iR22 = ((R11 * R33) - (R13 * R31)) / _det;
    float iR23 = ((R13 * R21) - (R11 * R23)) / _det;
    float iR31 = ((R21 * R32) - (R22 * R31)) / _det;
    float iR32 = ((R12 * R31) - (R11 * R32)) / _det;
    float iR33 = ((R11 * R22) - (R12 * R21)) / _det;

    Logger::getInstance().debug("[NiftiWriter] Inverted Bruker Rotation Matrix:");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(iR11) + ", " + toString(iR12) + ", " + toString(iR13) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(iR21) + ", " + toString(iR22) + ", " + toString(iR23) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(iR31) + ", " + toString(iR32) + ", " + toString(iR33) + ")");

    float _xOffset = (_readOffset * iR11) + (_phaseOffset * iR12) + (_sliceOffset * iR13);
    float _yOffset = (_readOffset * iR21) + (_phaseOffset * iR22) + (_sliceOffset * iR23);
    float _zOffset = (_readOffset * iR31) + (_phaseOffset * iR32) + (_sliceOffset * iR33);

//    float _xOffset = (_readOffset * R11) + (_phaseOffset * R12) + (_sliceOffset * R13);
//    float _yOffset = (_readOffset * R21) + (_phaseOffset * R22) + (_sliceOffset * R23);
//    float _zOffset = (_readOffset * R31) + (_phaseOffset * R32) + (_sliceOffset * R33);

    Logger::getInstance().debug("[NiftiWriter] calculated xyz offsets:");
    Logger::getInstance().debug("[NiftiWriter] x offset = " + toString(_xOffset));
    Logger::getInstance().debug("[NiftiWriter] y offset = " + toString(_yOffset));
    Logger::getInstance().debug("[NiftiWriter] z offset = " + toString(_zOffset));


    string _orientation     = slicePackOrientationParameter->getValue(0);
    string _readOrientation = slicePackReadOrientationParameter->getValue(0);

    float* _originalArray;
    float* _changeAxesArray;


    if(_orientation.compare("axial") == 0 || _orientation.compare("transversal") == 0) {
    	if(_readOrientation.compare("L_R") == 0) {
    		_originalArray    = (float[]){R11, R12, R13,   R21, R22, R23,   R31, R32, R33}; // axial_R_L / sagittal_A_P
    		_changeAxesArray  = (float[]){-1.0, 0.0, 0.0,   0.0, 1.0, 0.0,   0.0, 0.0, 1.0}; // axial_R_L
    	} else { // A_P
    		_originalArray    = (float[]){R12, R11, R13,   R22, R21, R23,   R32, R31, R33}; // !!! not tested yet
    		_changeAxesArray  = (float[]){1.0, 0.0, 0.0,   0.0, 1.0, 0.0,   0.0, 0.0, 1.0}; // !!! not tested yet
    	}
    }

    if(_orientation.compare("sagittal") == 0) {
    	if(_readOrientation.compare("H_F") == 0) {
    		_originalArray    = (float[]){R12, R11, R13,   R22, R21, R23,   R32, R31, R33}; // sagittal_H_F / coronal_H_F
    		_changeAxesArray  = (float[]){1.0, 0.0, 0.0,   0.0, -1.0, 0.0,   0.0, 0.0, -1.0}; // sagittal_H_F
    	} else { // A_P
       		_originalArray    = (float[]){R11, R12, R13,   R21, R22, R23,   R31, R32, R33}; // axial_R_L / sagittal_A_P
       	    _changeAxesArray  = (float[]){1.0, 0.0, 0.0,   0.0, 1.0, 0.0,   0.0, 0.0, -1.0}; // sagittal_A_P
    	}
    }

    if(_orientation.compare("coronal") == 0) {
    	if(_readOrientation.compare("H_F") == 0) {
    		_originalArray   = (float[]){R12, R11, R13,   R22, R21, R23,   R32, R31, R33}; // sagittal_H_F / coronal_H_F
    		_changeAxesArray = (float[]){1.0, 0.0, 0.0,   0.0, 1.0, 0.0,   0.0, 0.0, 1.0}; // coronal_H_F
    	} else { // L_R
    		_originalArray   = (float[]){R11, R12, R13,   R21, R22, R23,   R31, R32, R33}; // !!! not tested yet
    		_changeAxesArray = (float[]){1.0, 0.0, 0.0,   0.0, -1.0, 0.0,   0.0, 0.0, 1.0}; // !!! not tested yet
    	}
    }

    float _scalingArray[]    = {nHeader.pixdim[1], 0.0, 0.0,   0.0, nHeader.pixdim[2], 0.0,   0.0, 0.0, atof(slicePackSliceDistanceParameter->getValue(0).c_str())};

    vector<float> originalMatrix(_originalArray, _originalArray + 9);
    vector<float> scalingMatrix(_scalingArray, _scalingArray + 9);
    vector<float> changeAxesMatrix(_changeAxesArray, _changeAxesArray + 9);

    vector<float> scaledMatrix = matrixProduct(matrixProduct(originalMatrix, scalingMatrix), changeAxesMatrix);

    R11 = scaledMatrix[0];
    R12 = scaledMatrix[1];
    R13 = scaledMatrix[2];
    R21 = scaledMatrix[3];
    R22 = scaledMatrix[4];
    R23 = scaledMatrix[5];
    R31 = scaledMatrix[6];
    R32 = scaledMatrix[7];
    R33 = scaledMatrix[8];

    // dtp meaning "dicom to patient" as in spm_dicom_convert.m :-(

    float _dtp_offset_x = _readOffset  - (R11 * ((float)nHeader.dim[1] * 0.5 + 1.0)) - (R12 * ((float)nHeader.dim[2] * 0.5 + 1.0)) - (R13 * ((float)nHeader.dim[3] * 0.5 + 0.5));
    float _dtp_offset_y = _phaseOffset - (R21 * ((float)nHeader.dim[1] * 0.5 + 1.0)) - (R22 * ((float)nHeader.dim[2] * 0.5 + 1.0)) - (R23 * ((float)nHeader.dim[3] * 0.5 + 0.5));
    float _dtp_offset_z = _sliceOffset - (R31 * ((float)nHeader.dim[1] * 0.5 + 1.0)) - (R32 * ((float)nHeader.dim[2] * 0.5 + 1.0)) - (R33 * ((float)nHeader.dim[3] * 0.5 + 0.5));
//    float _dtp_offset_x = _xOffset  - (R11 * ((float)nHeader.dim[1] * 0.5 + 1.0)) - (R12 * ((float)nHeader.dim[2] * 0.5 + 1.0)) - (R13 * ((float)nHeader.dim[3] * 0.5 + 0.5));
//    float _dtp_offset_y = _yOffset - (R21 * ((float)nHeader.dim[1] * 0.5 + 1.0)) - (R22 * ((float)nHeader.dim[2] * 0.5 + 1.0)) - (R23 * ((float)nHeader.dim[3] * 0.5 + 0.5));
//    float _dtp_offset_z = _zOffset - (R31 * ((float)nHeader.dim[1] * 0.5 + 1.0)) - (R32 * ((float)nHeader.dim[2] * 0.5 + 1.0)) - (R33 * ((float)nHeader.dim[3] * 0.5 + 0.5));

//    float _dtp_offset_x = -_readOffset  - (R11 * (float)nHeader.dim[1] * 0.5) - (R12 * (float)nHeader.dim[2] * 0.5) - (R13 * (float)nHeader.dim[3] * 0.5);
//    float _dtp_offset_y = _phaseOffset - (R21 * (float)nHeader.dim[1] * 0.5) - (R22 * (float)nHeader.dim[2] * 0.5) - (R23 * (float)nHeader.dim[3] * 0.5);
//    float _dtp_offset_z = _sliceOffset - (R31 * (float)nHeader.dim[1] * 0.5) - (R32 * (float)nHeader.dim[2] * 0.5) - (R33 * (float)nHeader.dim[3] * 0.5);

    Logger::getInstance().debug("[NiftiWriter] calculated 'dicom_to_patient' offsets:");
    Logger::getInstance().debug("[NiftiWriter] dtp_offset_x = " + toString(_dtp_offset_x));
    Logger::getInstance().debug("[NiftiWriter] dtp_offset_y = " + toString(_dtp_offset_y));
    Logger::getInstance().debug("[NiftiWriter] dtp_offset_z = " + toString(_dtp_offset_z));

    // ptt -> "patient to tal" as in spm_dicom_convert.m
    // atd -> "analyze to dicom" as in spm_dicom_convert.m

    float _dtpArray[] = {R11, R12, R13, _dtp_offset_x,   R21, R22, R23, _dtp_offset_y,   R31, R32, R33, _dtp_offset_z,   0.0, 0.0, 0.0, 1.0};

    float _pttArray[] = {-1.0, 0.0, 0.0, 0.0,   0.0, -1.0, 0.0, 0.0,   0.0, 0.0, 1.0, 0.0,   0.0, 0.0, 0.0, 1.0};
    float _atdArray[] = { 1.0, 0.0, 0.0, 0.0,   0.0, -1.0, 0.0, (nHeader.dim[2] + 1.0),   0.0, 0.0, 1.0, 0.0,   0.0, 0.0, 0.0, 1.0};

    // from the beginning of encode_qform0.m -> "Convert from first voxel at [1,1,1] to first voxel at [0,0,0]"
    float _pixelShiftArray[] = {1.0, 0.0, 0.0, 1.0,   0.0, 1.0, 0.0, 1.0,   0.0, 0.0, 1.0, 1.0,   0.0, 0.0, 0.0, 1.0};

    vector<float> dtpMatrix(_dtpArray, _dtpArray + 16);

    vector<float> pttMatrix(_pttArray, _pttArray + 16);
    vector<float> atdMatrix(_atdArray, _atdArray + 16);

    vector<float> pixelShiftMatrix(_pixelShiftArray, _pixelShiftArray + 16);

    vector<float> correctedMatrix = matrixProduct(matrixProduct(matrixProduct(pttMatrix, dtpMatrix), atdMatrix), pixelShiftMatrix);

    R11 = correctedMatrix[0];
    R12 = correctedMatrix[1];
    R13 = correctedMatrix[2];
    R21 = correctedMatrix[4];
    R22 = correctedMatrix[5];
    R23 = correctedMatrix[6];
    R31 = correctedMatrix[8];
    R32 = correctedMatrix[9];
    R33 = correctedMatrix[10];

    Logger::getInstance().debug("[NiftiWriter] corrected Matrix for this crappy piece called SPM:");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R11) + ", " + toString(R12) + ", " + toString(R13) + ", " + toString(correctedMatrix[3]) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R21) + ", " + toString(R22) + ", " + toString(R23) + ", " + toString(correctedMatrix[7]) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R31) + ", " + toString(R32) + ", " + toString(R33) + ", " + toString(correctedMatrix[11]) + ")");

    float qoffset_x = correctedMatrix[3];
    float qoffset_y = correctedMatrix[7];
    float qoffset_z = correctedMatrix[11];

    Logger::getInstance().debug("[NiftiWriter] setting quaternion offsets to:");
    Logger::getInstance().debug("[NiftiWriter] qoffset_x = " + toString(qoffset_x));
    Logger::getInstance().debug("[NiftiWriter] qoffset_y = " + toString(qoffset_y));
    Logger::getInstance().debug("[NiftiWriter] qoffset_z = " + toString(qoffset_z));

    nHeader.qoffset_x = qoffset_x;
    nHeader.qoffset_y = qoffset_y;
    nHeader.qoffset_z = qoffset_z;

    Logger::getInstance().debug("[NiftiWriter] setting srow parameters to:");
    Logger::getInstance().debug("[NiftiWriter] srow_x = " + toString(R11) + " " + toString(R12) + " " + toString(R13) + " " + toString(qoffset_x));
    Logger::getInstance().debug("[NiftiWriter] srow_y = " + toString(R21) + " " + toString(R22) + " " + toString(R23) + " " + toString(qoffset_y));
    Logger::getInstance().debug("[NiftiWriter] srow_z = " + toString(R31) + " " + toString(R32) + " " + toString(R33) + " " + toString(qoffset_z));

    nHeader.srow_x[0] = R11;
    nHeader.srow_x[1] = R12;
    nHeader.srow_x[2] = R13;
    nHeader.srow_x[3] = qoffset_x;

    nHeader.srow_y[0] = R21;
    nHeader.srow_y[1] = R22;
    nHeader.srow_y[2] = R23;
    nHeader.srow_y[3] = qoffset_y;

    nHeader.srow_z[0] = R31;
    nHeader.srow_z[1] = R32;
    nHeader.srow_z[2] = R33;
    nHeader.srow_z[3] = qoffset_z;

    nHeader.sform_code = NIFTI_XFORM_SCANNER_ANAT;


    float _nonNormalizedArray[] = {R11, R12, R13,   R21, R22, R23,   R31, R32, R33};
    float _normalizerArray[] = {1.0 / nHeader.pixdim[1], 0.0, 0.0,   0.0, 1.0 / nHeader.pixdim[2], 0.0,   0.0, 0.0, 1.0 / atof(slicePackSliceDistanceParameter->getValue(0).c_str())};

    vector<float> nonNormalizedMatrix(_nonNormalizedArray, _nonNormalizedArray + 9);
    vector<float> normalizerMatrix(_normalizerArray, _normalizerArray + 9);

    vector<float> normalizedMatrix = matrixProduct(nonNormalizedMatrix, normalizerMatrix);

    R11 = normalizedMatrix[0];
    R12 = normalizedMatrix[1];
    R13 = normalizedMatrix[2];
    R21 = normalizedMatrix[3];
    R22 = normalizedMatrix[4];
    R23 = normalizedMatrix[5];
    R31 = normalizedMatrix[6];
    R32 = normalizedMatrix[7];
    R33 = normalizedMatrix[8];

    if((R11 * R22 * R33) - (R11 * R32 * R23) - (R21 * R12 * R33) + (R21 * R32 * R13) + (R31 * R12 * R23) - (R31 * R22 * R13) < 0.0) {
    	qfac = -1.0;
    	Logger::getInstance().debug("[NiftiWriter] setting qfac to " + toString(qfac));
    	R13 = -1.0 * R13;
    	R23 = -1.0 * R23;
    	R33 = -1.0 * R33;
    }

    Logger::getInstance().debug("[NiftiWriter] normalized Matrix:");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R11) + ", " + toString(R12) + ", " + toString(R13) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R21) + ", " + toString(R22) + ", " + toString(R23) + ")");
    Logger::getInstance().debug("[NiftiWriter] (" + toString(R31) + ", " + toString(R32) + ", " + toString(R33) + ")");

    // calculate quaternion elements
    //
    // the following source is taken from nifti1_io.c (as a wee reference)
    //
    //   =================================================================
    //
    //   /* now, compute quaternion parameters */
    //
    //   a = r11 + r22 + r33 + 1.0l ;
    //
    //   if( a > 0.5l ){                /* simplest case */
    //	   a = 0.5l * sqrt(a) ;
    //	   b = 0.25l * (r32-r23) / a ;
    //	   c = 0.25l * (r13-r31) / a ;
    //	   d = 0.25l * (r21-r12) / a ;
    //   } else {                       /* trickier case */
    //	   xd = 1.0 + r11 - (r22+r33) ;  /* 4*b*b */
    //	   yd = 1.0 + r22 - (r11+r33) ;  /* 4*c*c */
    //	   zd = 1.0 + r33 - (r11+r22) ;  /* 4*d*d */
    //	   if( xd > 1.0 ){
    //	     b = 0.5l * sqrt(xd) ;
    //	     c = 0.25l* (r12+r21) / b ;
    //	     d = 0.25l* (r13+r31) / b ;
    //	     a = 0.25l* (r32-r23) / b ;
    //	   } else if( yd > 1.0 ){
    //	     c = 0.5l * sqrt(yd) ;
    //	     b = 0.25l* (r12+r21) / c ;
    //	     d = 0.25l* (r23+r32) / c ;
    //	     a = 0.25l* (r13-r31) / c ;
    //	   } else {
    //	     d = 0.5l * sqrt(zd) ;
    //	     b = 0.25l* (r13+r31) / d ;
    //	     c = 0.25l* (r23+r32) / d ;
    //	     a = 0.25l* (r21-r12) / d ;
    //	   }
    //	   if( a < 0.0l ){ b=-b ; c=-c ; d=-d; a=-a; }
    //   }
    //
    //   =================================================================

    float quatern_a, quatern_b, quatern_c, quatern_d;
    float xd, yd, zd;

    quatern_a = 1.0 + R11 + R22 + R33;
    if(quatern_a > 0.5) {
      quatern_a = sqrt(quatern_a) / 2.0;
      quatern_b = (R32 - R23) / (4.0 * quatern_a);
      quatern_c = (R13 - R31) / (4.0 * quatern_a);
      quatern_d = (R21 - R12) / (4.0 * quatern_a);
    } else {
      xd = 1.0 + R11 - (R22 + R33);
      yd = 1.0 + R22 - (R11 + R33);
      zd = 1.0 + R33 - (R11 + R22);
      if(xd > 1.0) {
		quatern_b = sqrt(xd) / 2.0;
		quatern_c = (R12 + R21) / (4.0 * quatern_b);
		quatern_d = (R13 + R31) / (4.0 * quatern_b);
		quatern_a = (R32 - R23) / (4.0 * quatern_b);
      } else {
		if(yd > 1.0) {
		  quatern_c = sqrt(yd) / 2.0;
		  quatern_b = (R12 + R21) / (4.0 * quatern_c);
		  quatern_d = (R23 + R32) / (4.0 * quatern_c);
		  quatern_a = (R13 - R31) / (4.0 * quatern_c);
		} else {
		  quatern_d = sqrt(zd) / 2.0;
		  quatern_b = (R13 + R31) / (4.0 * quatern_d);
		  quatern_c = (R23 + R32) / (4.0 * quatern_d);
		  quatern_a = (R21 - R12) / (4.0 * quatern_d);
		}
      }
      if(quatern_a < 0.0) {
		quatern_a = -quatern_a;
		quatern_b = -quatern_b;
		quatern_c = -quatern_c;
		quatern_d = -quatern_d;
      }
    }

    Logger::getInstance().debug("[NiftiWriter] calculated quaternion elements for nifti header:");
    Logger::getInstance().debug("[NiftiWriter] quatern_a = " + toString(quatern_a));
    Logger::getInstance().debug("[NiftiWriter] quatern_b = " + toString(quatern_b));
    Logger::getInstance().debug("[NiftiWriter] quatern_c = " + toString(quatern_c));
    Logger::getInstance().debug("[NiftiWriter] quatern_d = " + toString(quatern_d));

    nHeader.qform_code = NIFTI_XFORM_SCANNER_ANAT;

    nHeader.quatern_b = quatern_b;
    nHeader.quatern_c = quatern_c;
    nHeader.quatern_d = quatern_d;

    nHeader.pixdim[0] = qfac;

  }

  vector<float> NiftiWriter::matrixProduct(vector<float> matrixA, vector<float> matrixB) {

	  if(matrixA.size() != matrixB.size()) {
		    Logger::getInstance().warning("[NiftiWriter] matrixProduct(): Input matrices are of different sizes. Returning empty vector.");
		    return vector<float>();
	  }

	  int _size = (int)sqrt((float)matrixA.size());

	  vector<float> result(_size * _size, 0.0);

	  for(int _j = 0; _j < _size; _j++) {
		  for(int _i = 0; _i < _size; _i++) {
			  for(int _sumIndex = 0; _sumIndex < _size; _sumIndex++) {
				  result[_j * _size + _i] += matrixA[_j * _size + _sumIndex] * matrixB[_sumIndex * _size + _i];
			  }
		  }
	  }

	  return result;
  }

}; // end of namespace nifti
