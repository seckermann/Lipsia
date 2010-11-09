#include "Configurator.h"

#include <stdio.h>

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

Configurator::Configurator(int *argc, char** argv)
{
	m_currInterpolator = NULL;
	setInterpolationtype(ip_nn);
	m_time = 0;
	parseVistaOptions(argc, argv);
	m_zoom = 1;
	setContrast(0);
	setBrightness(0);
	m_fps = 25;
	m_shift=0;
	m_factor=1;
	m_stretch=200;
	m_stretchfact=1.02;
}

Configurator::~Configurator()
{
}

void Configurator::setInterpolationtype(interpol_type t) {
	m_interpolationtype = t;
	if (m_currInterpolator!=NULL)
		delete m_currInterpolator;
	
	switch (t) {
	case ip_nn: m_currInterpolator = new CNnMag(); return;
	case ip_bilin: m_currInterpolator = new CBilinMag(); return;
	case ip_bicub: m_currInterpolator = new CBicubSplineMag(); return;
	case ip_bicub_6: m_currInterpolator = new CBicub6Mag(); return;
	case ip_bspline: m_currInterpolator = new CBSplineMag(); return;
	}
}

void Configurator::parseVistaOptions(int *argc, char** argv)
{
	VArgVector in_files;
	VBoolean in_found;
	
	double black = 0.01;
	double white = 0.01;
	
	VOptionDescRec options[] = { { "in", VStringRepn, 0, &in_files,
        &in_found, NULL, "Input file (VImage)" },
                   {"black",VDoubleRepn,1,(VPointer)&black,VOptionalOpt,NULL,"Lower histogram cut-off for contrast scaling in %"},
                   {"white",VDoubleRepn,1,(VPointer)&white,VOptionalOpt,NULL,"Upper histogram cut-off for contrast scaling in %"},};
	
/* 	
	if (!VParseCommand (VNumber (options), options, argc, argv)) {
		VReportBadArgs (*argc, argv);
		VReportUsage (argv[0], VNumber (options), options, NULL);
		exit (EXIT_FAILURE);
	}
	
	if (in_files.number==0) {
		VError("No file specified!\n");
		exit (EXIT_FAILURE);
	}
*/		

    /* Parse command line arguments and identify files: */
    if (! VParseCommand (VNumber (options), options,  argc, argv) ||
            ! VIdentifyFiles (VNumber (options), options, "in",  argc, argv, 0))
        goto Usage;
    if (*argc > 1) {
        VReportBadArgs (*argc, argv);
Usage:	VReportUsage (argv[0], VNumber (options), options, NULL);
        exit (EXIT_FAILURE);
    }

	VStringConst in_filename = ((VStringConst *) in_files.vector)[0];
	
	m_filename = QString(in_filename);
	FILE* fp = fopen(m_filename.latin1(), "r");
	if (fp==NULL) {
		fprintf(stderr, "Could not load file: %s\n", m_filename.latin1());
		exit (EXIT_FAILURE);
	}
	VAttrList in_list = VReadFile (fp, NULL);
	if (!in_list)
		exit (EXIT_FAILURE);
	fclose(fp);

	m_imgManager.setHistgrammCutOff(black, white);
	m_imgManager.init(in_list);
	VDestroyAttrList(in_list);
	
	setMousePosition(m_imgManager.bands()/2, m_imgManager.rows()/2, m_imgManager.cols()/2);
	lockPosition();
	
	setBrightness(0);
	setContrast(0);
}

void Configurator::lockPosition()
{
	m_band = m_mouseBand;
	m_row = m_mouseRow;
	m_col = m_mouseCol;
	
	m_imgManager.updateViewData(m_col, m_row, m_band, m_time);
	emit crossPositionChanged();
}

void Configurator::setPosition(int band, int row, int col)
{
	if ( (band>=0) && (band<m_imgManager.bands()))
		m_band = band;
	
	if ((row>=0) && (row<m_imgManager.rows()))
		m_row = row;
	
	if ((col>=0) && (col<m_imgManager.cols()))
		m_col = col;
	
	m_imgManager.updateViewData(m_col, m_row, m_band, m_time);
	emit crossPositionChanged();
}

void Configurator::setMousePosition(int band, int row, int col)
{
	if ( (band>=0) && (band<m_imgManager.bands()))
		m_mouseBand = band;
	
	if ((row>=0) && (row<m_imgManager.rows()))
		m_mouseRow = row;
	
	if ((col>=0) && (col<m_imgManager.cols()))
		m_mouseCol = col;

	emit mousePositionChanged();
}
