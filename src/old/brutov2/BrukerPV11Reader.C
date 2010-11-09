#include "BrukerPV11Reader.h"

/* global includes */
#include <iostream>
#include <fstream>
#include <ios>
#include <string>
#include <sstream>
#include <map>
#include <stdlib.h> 

/* local includes */
#include "Logger.h"
#include "IImageDataset.h"
#include "ImageDatasetParameter.h"
#include "Image.h"
#include "ConverterConfiguration.h"
#include "JCampDXParser.h"
#include "JCampDXUtilities.h"


using namespace converter;
using namespace jcampdx;
using namespace logger;
using namespace std;


namespace bruker
{

typedef map<string, Parameter> parameterTable;



class BrukerPV11BasedImageDataset : public converter::IImageDataset {
	public:
		BrukerPV11BasedImageDataset(BrukerPV11Reader* _reader) {
			reader = _reader;
		}
		
		// this looks a bit 'mickey mouse' but is necessary at some point
		ImageDatasetParameter* getParameter(string name) {
			if(name.compare(PATIENT_NAME) == 0) {
				return extractPatientName();
			} else if(name.compare(PATIENT_BIRTHDAY) == 0){
				return extractPatientBirthday();
			} else if(name.compare(PATIENT_SEX) == 0){
				return extractPatientSex();
			} else if(name.compare(AQUISITION_DATE) == 0){
				return extractAquisitionDate();
			} else if(name.compare(AQUISITION_TIME) == 0){
				return extractAquisitionTime();
			} else if(name.compare(DIMENSION_SIZE) == 0){
				return extractDimensionSize();
			} else if(name.compare(FIELD_OF_VIEW) == 0){
				return extractFieldOfView();
			} else if(name.compare(VOXEL_SIZE) == 0){
				return extractVoxelSize();
			} else if(name.compare(SLICE_THICKNESS) == 0){
				return extractSliceThickness();
			} else if(name.compare(SLICE_PACK_NUMBER) == 0){
				return extractSlicePackNumber();
			} else if(name.compare(SLICE_PACK_NUMBER_OF_SLICES) == 0){
				return extractSlicePackNumberOfSlices();
			} else if(name.compare(SLICE_PACK_POSITION) == 0){
				return extractSlicePackPosition();
			} else if(name.compare(SLICE_PACK_SLICE_GAP) == 0){
				return extractSlicePackSliceGap();
			} else if(name.compare(SLICE_PACK_SLICE_DISTANCE) == 0){
				return extractSlicePackSliceDistance();
			} else if(name.compare(SLICE_PACK_ORIENTATION) == 0){
				return extractSlicePackOrientation();
			} else if(name.compare(SLICE_PACK_READ_ORIENTATION) == 0){
				return extractSlicePackReadOrientation();
			} else if(name.compare(SLICE_PACK_AXES) == 0){
				return extractSlicePackAxes();
			} else if(name.compare(ECHO_TIME) == 0){
				return extractEchoTime();
			} else if(name.compare(NUMBER_OF_REPETITIONS) == 0){
				return extractNumberOfRepetitions();
			} else if(name.compare(REPETITION_TIME) == 0){
				return extractRepetitionTime();
			} else if(name.compare(SLICE_TIME) == 0){
				return extractSliceTime();
			} else if(name.compare(INVERSION_TIME) == 0){
				return extractInversionTime();
			} else if(name.compare(FLIP_ANGLE) == 0){
				return extractFlipAngle();
			} else if(name.compare(NUMBER_OF_A0_IMAGES) == 0){
				return extractNumberOfA0Images();
			} else if(name.compare(NUMBER_OF_DIFFUSION_DIRECTIONS) == 0){
				return extractNumberOfDiffusionDirections();
			} else if(name.compare(NUMBER_OF_EXPERIMENTS_IN_EACH_DIRECTION) == 0){
				return extractNumberOfExperimentsInEachDirection();
			} else if(name.compare(DIFFUSION_DIRECTIONS) == 0){
				return extractDiffusionDirections();
			} else if(name.compare(DATASET_TYPE) == 0){
				return analyzeDatasetType();
			} else if(name.compare(DATA_TYPE) == 0){
				return extractDataType();
			} else if(name.compare(MODALITY) == 0){
				return extractModality();
			} else if(name.compare(DEVICE) == 0){
				return extractDevice();
			} else if(name.compare(DEVICE_SOFTWARE) == 0){
				return extractDeviceSoftware();
			} else if(name.compare(COIL_ID) == 0){
				return extractCoilID();
			} else if(name.compare(METHOD_NAME) == 0){
				return extractMethodName();
			} else if(name.compare(PROTOCOL_NAME) == 0){
				return extractProtocolName();
			} else {
				return 0;
			}
		}
		
		Image* getImage(int index) {
			// TODO: we need to check the index (against the number of slices)

			string forceByteOrder;
			reader->configuration->getConfigurationValue("force.byteorder", forceByteOrder);


			string sourceFormat = extractBytesPerPixel();

			int _pixelByteSize;
			if(sourceFormat.compare(BrukerPV11Reader::SOURCE_FORMAT_SIGNED_BYTE) == 0 || sourceFormat.compare(BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_BYTE) == 0) {
				_pixelByteSize = 1;
			} else if(sourceFormat.compare(BrukerPV11Reader::SOURCE_FORMAT_SIGNED_SHORT) == 0 || sourceFormat.compare(BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_SHORT) == 0) {
				_pixelByteSize = 2;
			} else if(sourceFormat.compare(BrukerPV11Reader::SOURCE_FORMAT_SIGNED_INT) == 0 || sourceFormat.compare(BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_INT) == 0) {
				_pixelByteSize = 4;
			} else {
				// NOTE: this doesn't really make sense ... but heho ...
				_pixelByteSize = 1;
			}


			int width  = atoi(getParameter(DIMENSION_SIZE)->getValue(0).c_str());
			int height = atoi(getParameter(DIMENSION_SIZE)->getValue(1).c_str());
			float* _i = new float[width * height];
			int _imageByteSize = _pixelByteSize * width * height;
			//stringstream _dbgmsg;
			//_dbgmsg << "reading from recoDataFileStream -> " << reader->recoDataFileStream;
			//Logger::getInstance().debug(_dbgmsg.str());
			reader->recoDataFileStream->seekg(_imageByteSize * index, ios_base::beg);
			char _pixel0;
			char _pixel1;
			char _pixel2;
			char _pixel3;

			float _pixel;

            char* _imageByteBuffer = new char[_imageByteSize];
            int   _imageByteBufferCursor;
            
			switch(_pixelByteSize) {
				case 1: {
                    _imageByteBufferCursor = 0;
                    reader->recoDataFileStream->read(_imageByteBuffer, _imageByteSize);
					for(int y = 0; y < height; y++) {
                        int _y_times_width = y * width;
						for(int x = 0; x < width; x++) {
							//cout << "  stream position: " << recoDataFileStream->tellg() << endl;
                            _pixel0 = _imageByteBuffer[_imageByteBufferCursor++];
							_pixel = (float)(_pixel0 & 0xFF);
							_i[(_y_times_width) + x] = _pixel;
						}
					}
					break;
				}
				case 2: {
                    _imageByteBufferCursor = 0;
                    reader->recoDataFileStream->read(_imageByteBuffer, _imageByteSize);
                    bool _isBig = forceByteOrder.compare("big") == 0 || (forceByteOrder.compare("auto") == 0 && reader->recoParameters["RECO_byte_order"].getValue().compare("bigEndian") == 0); 
					for(int y = 0; y < height; y++) {
                        int _y_times_width = y * width;
						for(int x = 0; x < width; x++) {
							//cout << "  stream position: " << recoDataFileStream->tellg() << endl;
                            _pixel0 = _imageByteBuffer[_imageByteBufferCursor++];
                            _pixel1 = _imageByteBuffer[_imageByteBufferCursor++];
							if(_isBig) {
								_pixel = (float)((_pixel1 & 0xFF) | ((_pixel0 & 0xFF) << 8));
							} else {
								_pixel = (float)((_pixel0 & 0xFF) | ((_pixel1 & 0xFF) << 8));
							}
							_i[(_y_times_width) + x] = _pixel;
						}
					}
					break;
				}
				case 4: {
                    _imageByteBufferCursor = 0;
                    reader->recoDataFileStream->read(_imageByteBuffer, _imageByteSize);
                    bool _isBig = forceByteOrder.compare("big") == 0 || (forceByteOrder.compare("auto") == 0 && reader->recoParameters["RECO_byte_order"].getValue().compare("bigEndian") == 0); 
					for(int y = 0; y < height; y++) {
                        int _y_times_width = y * width;
						for(int x = 0; x < width; x++) {
							//cout << "  stream position: " << recoDataFileStream->tellg() << endl;
                            _pixel0 = _imageByteBuffer[_imageByteBufferCursor++];
                            _pixel1 = _imageByteBuffer[_imageByteBufferCursor++];
                            _pixel2 = _imageByteBuffer[_imageByteBufferCursor++];
                            _pixel3 = _imageByteBuffer[_imageByteBufferCursor++];
							if(_isBig) {
								_pixel = (float)((_pixel3 & 0xFF) | ((_pixel2 & 0xFF) << 8) | ((_pixel1 & 0xFF) << 16) | ((_pixel0 & 0xFF) << 24));
							} else {
								_pixel = (float)((_pixel0 & 0xFF) | ((_pixel1 & 0xFF) << 8) | ((_pixel2 & 0xFF) << 16) | ((_pixel3 & 0xFF) << 24));
							}
							_i[(_y_times_width) + x] = _pixel;
						}
					}
					break;
				}
			}
			
            delete[] _imageByteBuffer;
            
			return new Image(width, height, _i);
		}
		
	private:


		BrukerPV11Reader* reader;
	
		ImageDatasetParameter* extractPatientName() {
//			cout << "  [BrukerPV11BasedImageDataset] extractPatientName(): " << reader->subjectParameters["SUBJECT_name_string"].getValue() << endl;
			return new ImageDatasetParameter(PATIENT_NAME, reader->subjectParameters["SUBJECT_id"].getValue());
		}

		ImageDatasetParameter* extractPatientBirthday() {
//			cout << "  [BrukerPV11BasedImageDataset] extractPatientBirthday(): " << reader->subjectParameters["SUBJECT_dbirth"].getValue() << endl;
			return new ImageDatasetParameter(PATIENT_BIRTHDAY, reader->subjectParameters["SUBJECT_dbirth"].getValue());
		}

		ImageDatasetParameter* extractPatientSex() {
//			cout << "  [BrukerPV11BasedImageDataset] extractPatientSex(): " << reader->subjectParameters["SUBJECT_sex"].getValue() << endl;
			return new ImageDatasetParameter(PATIENT_SEX, reader->subjectParameters["SUBJECT_sex"].getValue());
		}

		ImageDatasetParameter* extractModality() {
			// for now modality is always 'MR'
			return new ImageDatasetParameter(MODALITY, "MR");
		}
		
		ImageDatasetParameter* extractDevice() {
			return new ImageDatasetParameter(DEVICE, "Bruker MedSpec");
		}

		ImageDatasetParameter* extractDeviceSoftware() {
			return new ImageDatasetParameter(DEVICE_SOFTWARE, "ParaVision 1.1.x");
		}

		ImageDatasetParameter* extractCoilID() {
			string coilID = reader->acqpParameters["ACQ_transmitter_coil"].getValue();
			if(coilID.size() > 0) {
				return new ImageDatasetParameter(COIL_ID, coilID);
			} else {
				return new ImageDatasetParameter(COIL_ID, "unknown");
			}
		}

		ImageDatasetParameter* extractMethodName() {
			string methodName = reader->acqpParameters["ACQ_method"].getValue();
			if(methodName.size() > 0) {
				return new ImageDatasetParameter(METHOD_NAME, methodName);
			} else {
				return new ImageDatasetParameter(METHOD_NAME, "unknown");
			}
		}

		ImageDatasetParameter* extractProtocolName() {
			string protocolName = reader->acqpParameters["ACQ_protocol_name"].getValue();
			if(protocolName.size() > 0) {
				return new ImageDatasetParameter(PROTOCOL_NAME, protocolName);
			} else {
				return new ImageDatasetParameter(PROTOCOL_NAME, "unknown");
			}
		}
		ImageDatasetParameter* extractAquisitionDate() {
			string _dateString = reader->subjectParameters["SUBJECT_date"].getValue();
			string::size_type _dateStart = _dateString.find_first_not_of(" ", 8);
			return new ImageDatasetParameter(AQUISITION_DATE, _dateString.substr(_dateStart, _dateString.size() - _dateStart));
		}

		ImageDatasetParameter* extractAquisitionTime() {
			return new ImageDatasetParameter(AQUISITION_TIME, reader->subjectParameters["SUBJECT_date"].getValue().substr(0, 8));
		}

		ImageDatasetParameter* extractDimensionSize() {

			string width;
			string height;
			string nSlicesString;

			if(reader->recoParameters.find("RECO_size") != reader->recoParameters.end()) {
				// first the image size (from reco)
				width  = reader->recoParameters["RECO_size"].getValue(0);
				height = reader->recoParameters["RECO_size"].getValue(1);
				
				if(reader->imndParameters["IMND_dimension"].getValue().compare("_3D") == 0) {
//		     		nSlicesString = reader->recoParameters["RECO_ft_size"].getValue(2);
					nSlicesString = reader->recoParameters["RECO_size"].getValue(2);
				} else { // we assume 2D
					int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
					int nSlices = 0;
					for(int i = 0; i < nSPacks; i++) {
						nSlices += atoi(reader->imndParameters["IMND_slicepack_n_slices"].getValue(i).c_str());
					}
					stringstream _ss;
					_ss << nSlices;
					_ss >> nSlicesString;
				}
			} else {
				// use d3proc parameters
				Logger::getInstance().warning("using 'd3proc' parameters to determine dimension sizes");
				width         = reader->d3procParameters["IM_SIX"].getValue(); 
				height        = reader->d3procParameters["IM_SIY"].getValue(); 
				nSlicesString = reader->d3procParameters["IM_SIZ"].getValue(); 
			}

			string fourthDimension;		
			if(analyzeDatasetType()->getValue().compare("DTI") == 0) {
				// this shouldn't happen with PV 11x

//				int nDiffDir = atoi(reader->imndParameters["PVM_DwNDiffDir"].getValue().c_str());
//				int nA0Imags = atoi(reader->imndParameters["PVM_DwAoImages"].getValue().c_str());
//				stringstream _fd;
//				_fd << (nDiffDir + nA0Imags);
//				_fd >> fourthDimension;
			} else {
				int nRepetitions = atoi(reader->acqpParameters["NR"].getValue().c_str());
				
				string repToConvertString;
				int    repToConvert;
				reader->configuration->getConfigurationValue("repetitions.to.convert", repToConvertString);
				repToConvert = atoi(repToConvertString.c_str());
				
				if(repToConvert == 0 || repToConvert > nRepetitions) {
					// 0 means all repetitions (but could still go wrong in the case of
					// an incomplete data file)
					// TODO: add robustness by either flagging an error if the data file size doesn't
					//       correspond to the parameters or simply just convert the _really_ existing
					//       repetitions
					stringstream _fd;
					_fd << (nRepetitions);
					_fd >> fourthDimension;
				} else {
					stringstream _fd;
					_fd << (repToConvert);
					_fd >> fourthDimension;
				}
				
			}
				
			vector<string> pValue(4);
			pValue[0] = height;
			pValue[1] = width;
			pValue[2] = nSlicesString;
			pValue[3] = fourthDimension;
			return new ImageDatasetParameter(DIMENSION_SIZE, pValue);
		}

		ImageDatasetParameter* extractFieldOfView() {
			if(reader->imndParameters["IMND_dimension"].getValue().compare("_3D") == 0) {
				vector<string> pValue(3);
				if(reader->recoParameters.find("RECO_fov") != reader->recoParameters.end()) {
					pValue[0] = reader->recoParameters["RECO_fov"].getValue(0);
					pValue[1] = reader->recoParameters["RECO_fov"].getValue(1);
					pValue[2] = reader->recoParameters["RECO_fov"].getValue(2);
				} else if(reader->imndParameters.find("MPIL_GEO_fov") != reader->imndParameters.end()) {
					Logger::getInstance().warning("no 'RECO_fov' parameter; using 'MPIL_GEO_fov' instead to determine FOV");
					pValue[0] = reader->imndParameters["MPIL_GEO_fov"].getValue(0);
					pValue[1] = reader->imndParameters["MPIL_GEO_fov"].getValue(1);
					pValue[2] = reader->imndParameters["MPIL_GEO_fov"].getValue(2);
				} else if(reader->imndParameters.find("IMND_fov") != reader->imndParameters.end()) {
					Logger::getInstance().warning("no 'RECO_fov' nor 'MPIL_GEO_fov' parameter; using 'IMND_fov' instead to determine FOV");
					pValue[0] = reader->imndParameters["IMND_fov"].getValue(0);
					pValue[1] = reader->imndParameters["IMND_fov"].getValue(1);
					pValue[2] = reader->imndParameters["IMND_fov"].getValue(2);
				} else {
					Logger::getInstance().warning("no FOV information found");
					pValue[0] = "unknown";
					pValue[1] = "unknown";
					pValue[2] = "unknown";
				}
				return new ImageDatasetParameter(FIELD_OF_VIEW, pValue);
			} else { // we assume 2D
				vector<string> pValue(2);
				if(reader->recoParameters.find("RECO_fov") != reader->recoParameters.end()) {
					pValue[0] = reader->recoParameters["RECO_fov"].getValue(0);
					pValue[1] = reader->recoParameters["RECO_fov"].getValue(1);
				} else if(reader->imndParameters.find("MPIL_GEO_fov") != reader->imndParameters.end()) {
					Logger::getInstance().warning("no 'RECO_fov' parameter; using 'MPIL_GEO_fov' instead to determine FOV");
					pValue[0] = reader->imndParameters["MPIL_GEO_fov"].getValue(0);
					pValue[1] = reader->imndParameters["MPIL_GEO_fov"].getValue(1);
				} else if(reader->imndParameters.find("IMND_fov") != reader->imndParameters.end()) {
					Logger::getInstance().warning("no 'RECO_fov' nor 'MPIL_GEO_fov' parameter; using 'IMND_fov' instead to determine FOV");
					pValue[0] = reader->imndParameters["IMND_fov"].getValue(0);
					pValue[1] = reader->imndParameters["IMND_fov"].getValue(1);
				} else {
					Logger::getInstance().warning("no FOV information found");
					pValue[0] = "unknown";
					pValue[1] = "unknown";
				}
				return new ImageDatasetParameter(FIELD_OF_VIEW, pValue);
			}
		}

		ImageDatasetParameter* extractVoxelSize() {
			vector<string> pValue(3);
			if(reader->recoParameters.find("RECO_fov") != reader->recoParameters.end()) {
				float _fovX  = atof(reader->recoParameters["RECO_fov"].getValue(0).c_str());
				float _fovY  = atof(reader->recoParameters["RECO_fov"].getValue(1).c_str());
				float _sizeX = atof(reader->recoParameters["RECO_size"].getValue(0).c_str());
				float _sizeY = atof(reader->recoParameters["RECO_size"].getValue(1).c_str());
				stringstream _vsX;
				_vsX << (_fovX * 10.0 / _sizeX);
				pValue[0] = _vsX.str();
				stringstream _vsY;
				_vsY << (_fovY * 10.0 / _sizeY);
				pValue[1] = _vsY.str();
			} else if(reader->imndParameters.find("MPIL_GEO_fov") != reader->imndParameters.end()) {
				Logger::getInstance().warning("no 'RECO_fov' parameter; using 'MPIL_GEO_fov' instead to determine voxel size");
				float _fovX  = atof(reader->imndParameters["MPIL_GEO_fov"].getValue(0).c_str());
				float _fovY  = atof(reader->imndParameters["MPIL_GEO_fov"].getValue(1).c_str());
				float _sizeX = atof(reader->d3procParameters["IM_SIX"].getValue().c_str());
				float _sizeY = atof(reader->d3procParameters["IM_SIY"].getValue().c_str());
				stringstream _vsX;
				_vsX << (_fovX * 10.0 / _sizeX);
				pValue[0] = _vsX.str();
				stringstream _vsY;
				_vsY << (_fovY * 10.0 / _sizeY);
				pValue[1] = _vsY.str();
			} else {
				Logger::getInstance().warning("no 'RECO_fov' nor 'MPIL_GEO_fov' parameter; using 'IMND_fov' instead to determine voxel size");
				float _fovX  = atof(reader->imndParameters["IMND_fov"].getValue(0).c_str());
				float _fovY  = atof(reader->imndParameters["IMND_fov"].getValue(1).c_str());
				float _sizeX = atof(reader->d3procParameters["IM_SIX"].getValue().c_str());
				float _sizeY = atof(reader->d3procParameters["IM_SIY"].getValue().c_str());
				stringstream _vsX;
				_vsX << (_fovX * 10.0 / _sizeX);
				pValue[0] = _vsX.str();
				stringstream _vsY;
				_vsY << (_fovY * 10.0 / _sizeY);
				pValue[1] = _vsY.str();
			}

			if(reader->imndParameters["IMND_dimension"].getValue().compare("_3D") == 0) {
				if(reader->recoParameters.find("RECO_fov") != reader->imndParameters.end()) {
					// 3D -> we've got a third FOV parameter and use it to calculate the third value for the voxel parameter
					float _fovZ  = atof(reader->recoParameters["RECO_fov"].getValue(2).c_str());
					float _sizeZ = atof(reader->recoParameters["RECO_size"].getValue(2).c_str());
					stringstream _vsZ;
					_vsZ << (_fovZ * 10.0 / _sizeZ);
					pValue[2] = _vsZ.str();
				} else if(reader->imndParameters.find("MPIL_GEO_fov") != reader->imndParameters.end()) {
					// the warning went out already  -  don't overstrain our users ;-)
					float _fovZ  = atof(reader->imndParameters["MPIL_GEO_fov"].getValue(2).c_str());
					float _sizeZ = atof(reader->d3procParameters["IM_SIZ"].getValue().c_str());
					stringstream _vsZ;
					_vsZ << (_fovZ * 10.0 / _sizeZ);
					pValue[2] = _vsZ.str();
				} else if(reader->imndParameters.find("IMND_fov") != reader->imndParameters.end()) {
					// the warning went out already (same as above)
					float _fovZ  = atof(reader->imndParameters["IMND_fov"].getValue(2).c_str());
					float _sizeZ = atof(reader->d3procParameters["IM_SIZ"].getValue().c_str());
					stringstream _vsZ;
					_vsZ << (_fovZ * 10.0 / _sizeZ);
					pValue[2] = _vsZ.str();
				}
			} else { // we assume 2D
				if(reader->imndParameters.find("MPIL_GEO_slicepack_gap") != reader->imndParameters.end()) {
					pValue[2] = reader->imndParameters["MPIL_GEO_slicepack_gap"].getValue();
				} else {
					Logger::getInstance().warning("no 'MPIL_GEO_slicepack_gap' parameter; using 'IMND_slicepack_gap' instead");
					pValue[2] = reader->imndParameters["IMND_slicepack_gap"].getValue();
				}
			}
			
			return new ImageDatasetParameter(VOXEL_SIZE, pValue);
		}

		ImageDatasetParameter* extractSliceThickness() {
			if(reader->imndParameters.find("MPIL_GEO_slice_thick") != reader->recoParameters.end()) {
				return new ImageDatasetParameter(SLICE_THICKNESS, reader->imndParameters["MPIL_GEO_slice_thick"].getValue());
			} else {
				Logger::getInstance().warning("no 'MPIL_GEO_slice_thick' parameter; using 'IMND_slice_thick' instead");
				return new ImageDatasetParameter(SLICE_THICKNESS, reader->imndParameters["IMND_slice_thick"].getValue());
			}
		}

		ImageDatasetParameter* extractSlicePackPosition() {
			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
			vector<string> pValue(nSPacks);
			for(int i = 0; i < nSPacks; i++) {
				pValue[i] = reader->imndParameters["IMND_slicepack_position"].getValue(i);
			}
			return new ImageDatasetParameter(SLICE_PACK_POSITION, pValue);
		}

		ImageDatasetParameter* extractSlicePackNumber() {
			return new ImageDatasetParameter(SLICE_PACK_NUMBER, reader->imndParameters["IMND_n_slicepacks"].getValue());
		}

		ImageDatasetParameter* extractSlicePackNumberOfSlices() {
			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
			vector<string> pValue(nSPacks);
			for(int i = 0; i < nSPacks; i++) {
				pValue[i] = reader->imndParameters["IMND_slicepack_n_slices"].getValue(i);
			}
			return new ImageDatasetParameter(SLICE_PACK_NUMBER_OF_SLICES, pValue);
		}

		ImageDatasetParameter* extractSlicePackSliceGap() {
			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
			vector<string> pValue(nSPacks);
			for(int i = 0; i < nSPacks; i++) {
				pValue[i] = reader->imndParameters["IMND_slicepack_gap"].getValue(i);
			}
			return new ImageDatasetParameter(SLICE_PACK_SLICE_GAP, pValue);
		}

		ImageDatasetParameter* extractSlicePackSliceDistance() {
			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
			vector<string> pValue(nSPacks);
			for(int i = 0; i < nSPacks; i++) {
				// TODO: check parameter - need distance not gap
				pValue[i] = reader->imndParameters["IMND_slicepack_gap"].getValue(i);
			}
			return new ImageDatasetParameter(SLICE_PACK_SLICE_DISTANCE, pValue);
		}

		ImageDatasetParameter* extractSlicePackOrientation() {
			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
			vector<string> pValue(nSPacks);
			for(int i = 0; i < nSPacks; i++) {
				// TODO: there is only one value in this parameter - see if there is a way around this HACK
				pValue[i] = reader->guiParameters["GUI_slice_dir_iso"].getValue();
			}
			return new ImageDatasetParameter(SLICE_PACK_ORIENTATION, pValue);
		}

		ImageDatasetParameter* extractSlicePackReadOrientation() {
			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
			vector<string> pValue(nSPacks);
			for(int i = 0; i < nSPacks; i++) {
				pValue[i] = "unknown";//reader->imndParameters["PVM_SPackArrReadOrient"].getValue(i);
			}
			return new ImageDatasetParameter(SLICE_PACK_READ_ORIENTATION, pValue);
		}

		ImageDatasetParameter* extractSlicePackAngle() {
//			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
//			vector<string> pValue(nSPacks * 9);
//			for(int i = 0; i < nSPacks; i++) {
//				for(int j = 0; j < 9; j++) {
//					pValue[i * 9 + j] = reader->imndParameters["PVM_SPackArrGradOrient"].getValue(i * 9 + j);
//				}
//			}
			return extractSlicePackAxes();
//			return new ImageDatasetParameter(SLICE_PACK_ANGLE, pValue);
		}

		ImageDatasetParameter* extractEchoTime() {
			return new ImageDatasetParameter(ECHO_TIME, reader->imndParameters["IMND_echo_time"].getValue());
		}

		ImageDatasetParameter* extractNumberOfRepetitions() {
			return new ImageDatasetParameter(NUMBER_OF_REPETITIONS, reader->acqpParameters["NR"].getValue());
		}

		ImageDatasetParameter* extractRepetitionTime() {
			if(reader->imndParameters.find("MPIL_repetition_time") != reader->imndParameters.end()) {
				return new ImageDatasetParameter(REPETITION_TIME, reader->imndParameters["MPIL_repetition_time"].getValue());
			} else if(reader->acqpParameters.find("ACQ_repetition_time") != reader->acqpParameters.end()) {
				return new ImageDatasetParameter(REPETITION_TIME, reader->acqpParameters["ACQ_repetition_time"].getValue());
			} else {
				return new ImageDatasetParameter(REPETITION_TIME, "unknown");
			}
		}

		ImageDatasetParameter* extractSliceTime() {
			stringstream _nsss(extractDimensionSize()->getValue(2));
			double nSlices;
			_nsss >> nSlices;
			
			vector<string> sliceTime((int)nSlices);

			if(reader->imndParameters.find("MPIL_slice_time_array") != reader->imndParameters.end()) {
				for(int counter = 0; counter < nSlices; counter++) {
					stringstream _stss;
					_stss.str(reader->imndParameters["MPIL_slice_time_array"].getValue(counter));
					sliceTime[counter] = string(_stss.str());
				}
			} else {
				for(int counter = 0; counter < nSlices; counter++) {
					sliceTime[counter] = string("unknown");
				}
			}

			return new ImageDatasetParameter(SLICE_TIME, sliceTime);
		}

		ImageDatasetParameter* extractInversionTime() {
			if(reader->imndParameters.find("MPIL_inversion_time") != reader->imndParameters.end()) {
				Logger::getInstance().warning("using MPIL parameter: MPIL_inversion_time");
				return new ImageDatasetParameter(INVERSION_TIME, reader->imndParameters["MPIL_inversion_time"].getValue());
			} else if(reader->acqpParameters.find("ACQ_inversion_time") != reader->acqpParameters.end()) {
				return new ImageDatasetParameter(INVERSION_TIME, reader->acqpParameters["ACQ_inversion_time"].getValue());
			} else {
				return new ImageDatasetParameter(INVERSION_TIME, "unknown");
			}
		}

		ImageDatasetParameter* extractFlipAngle() {
			if(reader->imndParameters.find("MPIL_excitation_pulse") != reader->imndParameters.end()) {
				return new ImageDatasetParameter(FLIP_ANGLE, reader->imndParameters["MPIL_excitation_pulse"].getValue(0));
			} else if(reader->acqpParameters.find("ACQ_flip_angle") != reader->acqpParameters.end()) {
				return new ImageDatasetParameter(FLIP_ANGLE, reader->acqpParameters["ACQ_flip_angle"].getValue());
			} else {
				return new ImageDatasetParameter(FLIP_ANGLE, "unknown");
			}
		}

		/*
		 * DTI related parameters
		 */

		ImageDatasetParameter* extractNumberOfA0Images() {
			return new ImageDatasetParameter(NUMBER_OF_A0_IMAGES, reader->imndParameters["PVM_DwAoImages"].getValue());
		}

		ImageDatasetParameter* extractNumberOfDiffusionDirections() {
			return new ImageDatasetParameter(NUMBER_OF_DIFFUSION_DIRECTIONS, reader->imndParameters["PVM_DwNDiffDir"].getValue());
		}

		ImageDatasetParameter* extractNumberOfExperimentsInEachDirection() {
			return new ImageDatasetParameter(NUMBER_OF_EXPERIMENTS_IN_EACH_DIRECTION, reader->imndParameters["PVM_DwNDiffExpEach"].getValue());
		}

		ImageDatasetParameter* extractDiffusionDirections() {
			int nDiffDir = atoi(reader->imndParameters["PVM_DwNDiffDir"].getValue().c_str());
			vector<string> pValue(nDiffDir * 3);
			for(int i = 0; i < nDiffDir * 3; i++) {
				pValue[i] = reader->imndParameters["PVM_DwDir"].getValue(i);
			}
			return new ImageDatasetParameter(DIFFUSION_DIRECTIONS, pValue);
		}


		ImageDatasetParameter* analyzeDatasetType() {


			int _nRep;
			stringstream _nrss(reader->acqpParameters["NR"].getValue());
			_nrss >> _nRep;
			
			string ff, fa;
			reader->configuration->getConfigurationValue("force.functional", ff);
			reader->configuration->getConfigurationValue("force.anatomical", fa);
			bool _forceFunctional = (ff.compare("yes") == 0);
			bool _forceAnatomical = (fa.compare("yes") == 0);
			if(!_forceAnatomical && (_nRep > 10 || _forceFunctional)) {
				return new ImageDatasetParameter(DATASET_TYPE, "FUNCTIONAL");
			} else {
				return new ImageDatasetParameter(DATASET_TYPE, "ANATOMICAL");
			}				
		}
		
		ImageDatasetParameter* extractSlicePackAxes() {
			int nSPacks = atoi(reader->imndParameters["IMND_n_slicepacks"].getValue().c_str());
			
			vector<float>  gradientOrientation(nSPacks * 9);
			vector<string> readOrientation(nSPacks);
			vector<string> sliceOrientation(nSPacks);
			for(int i = 0; i < nSPacks; i++) {
//				readOrientation[i]  = reader->imndParameters["PVM_SPackArrReadOrient"].getValue(i);
//				sliceOrientation[i] = reader->imndParameters["PVM_SPackArrSliceOrient"].getValue(i);
				for(int j = 0; j < 9; j++) {
					gradientOrientation[i * 9 + j] = atof(reader->acqpParameters["ACQ_grad_matrix"].getValue(i * 9 + j).c_str());
				}
			}

//			for(int i = 0; i < nSPacks; i++) {
//				if((sliceOrientation[i].compare("axial")    == 0 && readOrientation[i].compare("A_P") == 0) ||
//				   (sliceOrientation[i].compare("sagittal") == 0 && readOrientation[i].compare("A_P") == 0) ||
//				   (sliceOrientation[i].compare("coronal")  == 0 && readOrientation[i].compare("L_R") == 0)) {
//				   	vector<float> _go(gradientOrientation.begin() + (i * 9), gradientOrientation.begin() + ((i + 1) * 9));
//					Logger::getInstance().debug("... correcting matrix");
//					Logger::getInstance().debug("Old gradOrient matrix");
//					debugPrintMatrix(_go);
//				   	float _cmElements[] = { 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
//				   	vector<float> _cm(_cmElements, _cmElements + 9);
//					vector<float> corrected = matMultiply(_cm, _go);
//					Logger::getInstance().debug("Corrected gradOrient matrix");
//					debugPrintMatrix(corrected);
//				   	float _xrotpi[] = { 1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0 };
//				   	vector<float> _xr(_xrotpi, _xrotpi + 9);
//					vector<float> rotated = matMultiply(corrected, _xr);
//					Logger::getInstance().debug("Rotated gradOrient matrix");
//					debugPrintMatrix(rotated);
//					for(int j = 0; j < 9; j++) {
//						gradientOrientation[i * 9 + j] = rotated[j];
//					}
//				}
//			}

			vector<string> stringGO(nSPacks * 9);
			for(int i = 0; i < nSPacks; i++) {
				for(int j = 0; j < 9; j++) {
					stringstream _f;
					_f << gradientOrientation[i * 9 + j];
					stringGO[i * 9 + j] = _f.str();
				}
			}

			return new ImageDatasetParameter(SLICE_PACK_AXES, stringGO);
		}

		void debugPrintMatrix(vector<float> m) {
			for(int i = 0; i < 3; i++) {
				stringstream _row;
				_row << m[i * 3] << "  " << m[i * 3 + 1] << "  " << m[i * 3 + 2];
				Logger::getInstance().debug(_row.str());
			}
		}
		
		// the sole purpose of this method is to multiply one 3x3 matrix (in the form
		// of a vector containing 9 elements row-major) with another 3x3 matrix (in the same format)
		// ... I simply don't want to implement any proper c++ matrix stuff ... for now :-)
		vector<float> matMultiply(vector<float> A, vector<float> B) {
			vector<float> product(9);
			
			product[0] = A[0]*B[0] + A[1]*B[3] + A[2]*B[6];
			product[1] = A[0]*B[1] + A[1]*B[4] + A[2]*B[7];
			product[2] = A[0]*B[2] + A[1]*B[5] + A[2]*B[8];
			product[3] = A[3]*B[0] + A[4]*B[3] + A[5]*B[6];
			product[4] = A[3]*B[1] + A[4]*B[4] + A[5]*B[7];
			product[5] = A[3]*B[2] + A[4]*B[5] + A[5]*B[8];
			product[6] = A[6]*B[0] + A[7]*B[3] + A[8]*B[6];
			product[7] = A[6]*B[1] + A[7]*B[4] + A[8]*B[7];
			product[8] = A[6]*B[2] + A[7]*B[5] + A[8]*B[8];
			
			return product;
		}
		
		ImageDatasetParameter* extractDataType() {
			return new ImageDatasetParameter(DATA_TYPE, extractBytesPerPixel());
		}

		// extract the actual image data byte format
		string extractBytesPerPixel() {
			if(reader->recoParameters.find("RECO_wordtype") != reader->recoParameters.end()) {
				string _wType = reader->recoParameters["RECO_wordtype"].getValue();
				if(_wType.compare("_16BIT_SGN_INT") == 0) {
//					stringstream _ss;
//					_ss << "data byte format: " << BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_SHORT;
//					Logger::getInstance().debug(_ss.str());
					return BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_SHORT;
				} else {
//					stringstream _ss;
//					_ss << "data byte format: " << BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_INT;
//					Logger::getInstance().debug(_ss.str());
					return BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_INT;
				}
			} else {
				stringstream _dtss(reader->imndParameters["DATTYPE"].getValue());
				int datType;
				_dtss >> datType;
				switch(datType) {
					case 0: {
						return BrukerPV11Reader::SOURCE_FORMAT_BIT;
						break;
					}
					case 1: {
						return BrukerPV11Reader::SOURCE_FORMAT_SIGNED_BYTE;
						break;
					}
					case 2: {
						return BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_BYTE;
						break;
					}
					case 3: {
						return BrukerPV11Reader::SOURCE_FORMAT_SIGNED_SHORT;
						break;
					}
					case 4: {
						return BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_SHORT;
						break;
					}
					case 5: {
						return BrukerPV11Reader::SOURCE_FORMAT_SIGNED_INT;
						break;
					}
					case 6: {
						return BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_INT;
						break;
					}
					default: {
						return "source.format.unknown";
					}
				}
			}
		}
};

BrukerPV11Reader::BrukerPV11Reader(ConverterConfiguration* cc) {
	configuration = cc;

	// find the root directory of this dataset
	configuration->getConfigurationValue(ConverterConfiguration::SOURCE_URL_KEY, rootDirectory);
	// for now we only deal with absolute! 'file' URL's
	// this means it should start with 'file:' followed by _at_least_ one '/'
	// so we look for the colon, then the first non-'/' and erase this from the
	// string except the last '/' ('file:///home/foo/bar/dataset' -> '/home/foo/bar/dataset'
	// TODO: we need some error handling in case the URL is wrong 
//	rootDirectory.erase(0, rootDirectory.find_first_not_of("/", rootDirectory.find_first_of(':') + 1) - 1);
	rootDirectory.erase(0, rootDirectory.find_first_of(':') + 1);
	// cout << " [BrukerPV11Reader] rootDirectory: " << rootDirectory << endl;
	Logger::getInstance().debug("root directory: " + rootDirectory);

	// get exp number, reco number, reco data file name	
	string experimentNumber, recoNumber, recoDataFileName;
	configuration->getConfigurationValue("bruker.experiment.number", experimentNumber);
	configuration->getConfigurationValue("bruker.experiment.reco.number", recoNumber);
	configuration->getConfigurationValue("bruker.experiment.reco.data.file", recoDataFileName);

	Logger::getInstance().debug("experimentNumber: " + experimentNumber);
	Logger::getInstance().debug("recoNumber: " + recoNumber);
	Logger::getInstance().debug("recoDataFileName: " + recoDataFileName);

	Logger::getInstance().debug("reading bruker experiment " + rootDirectory + "/" + experimentNumber + "/" + recoNumber + "/" + recoDataFileName);

	// get a parser
	JCampDXParser* jcampdxParser = new JCampDXParser();
	// parse the 'subject' file first
	ifstream subjectStream((rootDirectory + "/" + "subject").c_str());
	jcampdxParser->parse(subjectStream, &subjectParameters);
	subjectStream.close();
	
	parameterTable::iterator spIterator;
	int spCounter = 1;
	for(spIterator = subjectParameters.begin(); spIterator != subjectParameters.end(); spIterator++, spCounter++) {
		stringstream _pss;
		_pss << spCounter << ":  " <<  spIterator->first << "=" << spIterator->second.getValue();
		Logger::getInstance().debug("  " + _pss.str());
	}

	// take the reco number and parse the 'd3proc' file
	ifstream d3procStream((rootDirectory + "/" + experimentNumber + "/pdata/" + recoNumber + "/d3proc").c_str());
	jcampdxParser->parse(d3procStream, &d3procParameters);
	d3procStream.close();

	parameterTable::iterator d3pIterator;
	int d3pCounter = 1;
	for(d3pIterator = d3procParameters.begin(); d3pIterator != d3procParameters.end(); d3pIterator++, d3pCounter++) {
		stringstream _pss;
		_pss << d3pCounter << ":  " <<  d3pIterator->first << "=" << d3pIterator->second.getValue();
		Logger::getInstance().debug("  " + _pss.str());
	}

	// take the reco number and parse the 'reco' file
	ifstream recoStream((rootDirectory + "/" + experimentNumber + "/pdata/" + recoNumber + "/reco").c_str());
	jcampdxParser->parse(recoStream, &recoParameters);
	recoStream.close();

	parameterTable::iterator rpIterator;
	int rpCounter = 1;
	for(rpIterator = recoParameters.begin(); rpIterator != recoParameters.end(); rpIterator++, rpCounter++) {
		stringstream _pss;
		_pss << rpCounter << ":  " <<  rpIterator->first << "=" << rpIterator->second.getValue();
		Logger::getInstance().debug("  " + _pss.str());
	}

	// take the exp number and parse the 'imnd' file
	ifstream imndStream((rootDirectory + "/" + experimentNumber + "/imnd").c_str());
	jcampdxParser->parse(imndStream, &imndParameters);
	imndStream.close();

	parameterTable::iterator ipIterator;
	int ipCounter = 1;
	for(ipIterator = imndParameters.begin(); ipIterator != imndParameters.end(); ipIterator++, ipCounter++) {
		stringstream _pss;
		_pss << ipCounter << ":  " <<  ipIterator->first << "=" << ipIterator->second.getValue();
		Logger::getInstance().debug("  " + _pss.str());
	}

	// take the exp number and parse the 'acqp' file
	ifstream acqpStream((rootDirectory + "/" + experimentNumber + "/acqp").c_str());
	jcampdxParser->parse(acqpStream, &acqpParameters);
	acqpStream.close();

	parameterTable::iterator apIterator;
	int apCounter = 1;
	for(apIterator = acqpParameters.begin(); apIterator != acqpParameters.end(); apIterator++, apCounter++) {
		stringstream _pss;
		_pss << apCounter << ":  " <<  apIterator->first << "=" << apIterator->second.getValue();
		Logger::getInstance().debug("  " + _pss.str());
	}

	// take the exp number and parse the 'gui' file
	ifstream guiStream((rootDirectory + "/" + experimentNumber + "/gui").c_str());
	jcampdxParser->parse(guiStream, &guiParameters);
	guiStream.close();

	parameterTable::iterator gpIterator;
	int gpCounter = 1;
	for(gpIterator = guiParameters.begin(); gpIterator != guiParameters.end(); gpIterator++, gpCounter++) {
		stringstream _pss;
		_pss << gpCounter << ":  " <<  gpIterator->first << "=" << gpIterator->second.getValue();
		Logger::getInstance().debug("  " + _pss.str());
	}


	recoDataFileStream = new ifstream((rootDirectory + "/" + experimentNumber + "/pdata/" + recoNumber + "/" + recoDataFileName).c_str());
	Logger::getInstance().debug("recoDataFileStream -> " + rootDirectory + "/" + experimentNumber + "/pdata/" + recoNumber + "/" + recoDataFileName);
	stringstream _dbgmsg;
	_dbgmsg << "recoDataFileStream -> " << recoDataFileStream;
	Logger::getInstance().debug(_dbgmsg.str());
}

BrukerPV11Reader::~BrukerPV11Reader() {
}

IImageDataset* BrukerPV11Reader::read() {

	IImageDataset* dataset = new BrukerPV11BasedImageDataset(this);
	
	// debug
//	dataset->getParameter(PATIENT_NAME);
//	dataset->getParameter(PATIENT_BIRTHDAY);
//	dataset->getParameter(PATIENT_SEX);
//	dataset->getParameter(AQUISITION_DATE);
//	dataset->getParameter(AQUISITION_TIME);
//	dataset->getParameter(DIMENSION_SIZE);
//	dataset->getParameter(FIELD_OF_VIEW);
//	dataset->getParameter(SLICE_THICKNESS);
//	dataset->getParameter(SLICE_PACK_POSITION);
//	dataset->getParameter(SLICE_PACK_ORIENTATION);
//	dataset->getParameter(SLICE_PACK_READ_ORIENTATION);
//	dataset->getParameter(SLICE_PACK_ANGLE);
//	dataset->getParameter(ECHO_TIME);
//	dataset->getParameter(NUMBER_OF_REPETITIONS);
//	dataset->getParameter(REPETITION_TIME);
//	dataset->getParameter(INVERSION_TIME);
	// debug end
	
    string forceByteOrder;
    configuration->getConfigurationValue("force.byteorder", forceByteOrder);
    
    if(recoParameters["RECO_byte_order"].getValue().compare("littleEndian") == 0 && forceByteOrder.compare("big") != 0) {
        Logger::getInstance().warning("ParaVision 1.1.x datasets are usually in 'big endian' format but the RECO_byte_order parameter is set to 'little endian'.");
        Logger::getInstance().warning("Please review the converted images and if they look like plain noise consider running brutov2 with the option '-fbo big' to");
        Logger::getInstance().warning("enforce the use of 'big endian' byte order.");
    }
	
	return dataset;
}

	const string BrukerPV11Reader::SOURCE_FORMAT_BIT             = "source.format.bit";
	const string BrukerPV11Reader::SOURCE_FORMAT_SIGNED_BYTE     = "source.format.signed.byte";
	const string BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_BYTE   = "source.format.unsigned.byte";
	const string BrukerPV11Reader::SOURCE_FORMAT_SIGNED_SHORT    = "source.format.signed.short";
	const string BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_SHORT  = "source.format.unsigned.short";
	const string BrukerPV11Reader::SOURCE_FORMAT_SIGNED_INT      = "source.format.signed.int";
	const string BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_INT    = "source.format.unsigned.int";
	const string BrukerPV11Reader::SOURCE_FORMAT_SIGNED_LONG     = "source.format.signed.long";
	const string BrukerPV11Reader::SOURCE_FORMAT_UNSIGNED_LONG   = "source.format.unsigned.long";



};
