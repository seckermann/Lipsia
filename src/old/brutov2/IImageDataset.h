#ifndef _IIMAGEDATASET_H_
#define _IIMAGEDATASET_H_

/* global includes */
#include <string>

/* local includes */
#include "ImageDatasetParameter.h"
#include "Image.h"


using namespace std;


namespace converter
{

/**
 * The 'mother' of all image dataset implementations.
 * <p>
 * IImageDataset usually gets implemented by the various
 * reader implementations, thus allowing for reader specific
 * optimizations.
 * <p>
 * The writer class then uses the getParameter() and getImage()
 * methods to create the convertet dataset. The writer can always
 * rely on the existence of certain general parameters. For other
 * parameters (dataset type specific ones) the writer has to implement
 * some sort of error handling to make sure the system doesn't break.
 * <p>
 * The following general parameters will always exist:
 * 
 * DATASET_DIMENSION_SIZE - a vector of int values giving the size
 *                          of the dataset in each dimension the
 *                          first dimension is always the number
 *                          of rows, the second dimension is the
 *                          number of columns, and the following
 *                          dimensions depend on the kind of dataset
 * 
 *  ....
 * 
 */
class IImageDataset
{
	
public:
	
	virtual ImageDatasetParameter*  getParameter(string name) = 0;
	virtual Image*                  getImage(int index)       = 0;
	
};

	// general parameters

	/* width, height, n.slices */
	static const string DATASET_TYPE("dataset.type");
	static const string DATA_TYPE("data.type");
	static const string DIMENSION_SIZE("dimension.size");
	static const string FIELD_OF_VIEW("field.of.view");
	static const string VOXEL_SIZE("voxel.size");
	static const string SLICE_THICKNESS("slice.thickness");
	/* number of slice packs */
	static const string SLICE_PACK_NUMBER("slice.pack.number");
	/* array with number of slices inside each slice packs */
	static const string SLICE_PACK_NUMBER_OF_SLICES("slice.pack.number.of.slices");
	/* position of the center of the slice pack - array with positions of all packs */
	static const string SLICE_PACK_POSITION("slice.pack.position");
	/* offset of the center of the slice pack - array with offsets of all packs */
	static const string SLICE_PACK_OFFSET("slice.pack.offset");
	/* gap within the slice pack - array with slice gaps of all packs */
	static const string SLICE_PACK_SLICE_GAP("slice.pack.slice.gap");
	/* distance within the slice pack - array with slice distances of all packs */
	static const string SLICE_PACK_SLICE_DISTANCE("slice.pack.slice.distance");
	/* orientation of the slices in the slice pack (axial, coronal, ...) */
	static const string SLICE_PACK_ORIENTATION("slice.pack.orientation");
	/* read orientation of the slice pack (something like L_R) */
	static const string SLICE_PACK_READ_ORIENTATION("slice.pack.read.orientation");
	/* axes of the slice pack */
	static const string SLICE_PACK_AXES("slice.pack.axes");
	/* echo time */
	static const string ECHO_TIME("echo.time");
	/* number of repetitions */
	static const string NUMBER_OF_REPETITIONS("number.of.repetitions");
	/* repetition time */
	static const string REPETITION_TIME("repetition.time");
	/* slice time */
	static const string SLICE_TIME("slice.time");
	/* inversion time */
	static const string INVERSION_TIME("inversion.time");
	/* flip angle */
	static const string FLIP_ANGLE("flip.angle");
	
	/* 
	 * the next two thingies are temporary until the method is
	 * settled enough
	 */
	static const string TMP_NUMBER_OF_ECHOES("tmp.number.of.echoes");
	static const string TMP_ECHO_TIME_ARRAY("tmp.echo.time.array");

	static const string NUMBER_OF_A0_IMAGES("number.of.a0.images");
	static const string NUMBER_OF_DIFFUSION_DIRECTIONS("number.of.diffusion.directions");
	static const string NUMBER_OF_EXPERIMENTS_IN_EACH_DIRECTION("number.of.experiments.in.each.direction");
	static const string DIFFUSION_DIRECTIONS("diffusion.directions");
	static const string DIFFUSION_B_VALUES("diffusion.b.values");
	static const string DIFFUSION_EFFECTIVE_B_VALUES("diffusion.effective.b.values");
	
	static const string PATIENT_NAME("patient.name");
	static const string PATIENT_BIRTHDAY("patient.birthday");
	static const string PATIENT_SEX("patient.sex");
	static const string AQUISITION_DATE("aquisition.date");
	static const string AQUISITION_TIME("aquisition.time");
	static const string MODALITY("modality");
	static const string DEVICE("device");
	static const string DEVICE_SOFTWARE("device.software");
	static const string COIL_ID("coil.id");
	static const string METHOD_NAME("method.name");
	static const string PROTOCOL_NAME("protocol.name");

};

#endif /*_IIMAGEDATASET_H_*/
