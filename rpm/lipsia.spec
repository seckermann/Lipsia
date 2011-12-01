Summary: Lipsia 
Name: Lipsia
Version: 2.2.0
Release: 0
License: GPL
Group: Applications/Engineering
%description 
	Lipsia
%files
/usr/bin/niftov
/usr/bin/v2ndlevel
/usr/bin/vaddcovariates
/usr/bin/valign3d
/usr/bin/valphasim
/usr/bin/vanonym
/usr/bin/varfit
/usr/bin/vattrcopy
/usr/bin/vattredit
/usr/bin/vave
/usr/bin/vbayes
/usr/bin/vbayesgroup
/usr/bin/vblobsize
/usr/bin/vbrainmask
/usr/bin/vbrainpeel
/usr/bin/vcacp
/usr/bin/vcatdesign
/usr/bin/vccm
/usr/bin/vcheckdata
/usr/bin/vcolorglm
/usr/bin/vcompcor
/usr/bin/vconvcoord
/usr/bin/vcorr
/usr/bin/vcovariates
/usr/bin/vcreatemask
/usr/bin/vdeform
/usr/bin/vdelcereb
/usr/bin/vdemon
/usr/bin/vdenoise
/usr/bin/vdespike
/usr/bin/vdifftimecourse
/usr/bin/vdistortionCorrection
/usr/bin/vdomulticomp
/usr/bin/vdotrans
/usr/bin/vdotrans3d
/usr/bin/vecm
/usr/bin/veta2
/usr/bin/vextractparam
/usr/bin/vfdr
/usr/bin/vfunctrans
/usr/bin/vgauss
/usr/bin/vgaussianize
/usr/bin/vgendesign
/usr/bin/vgen_wilcoxtable
/usr/bin/vgetcontrast
/usr/bin/vgetcovariates
/usr/bin/vglobalmean
/usr/bin/vimagemask
/usr/bin/vimagetimecourse
/usr/bin/visotrop
/usr/bin/vmapscale
/usr/bin/vmaskave
/usr/bin/vmaskedit
/usr/bin/vmetropolis
/usr/bin/vmovcorrection
/usr/bin/vmovcorrection2d
/usr/bin/vmulticomp
/usr/bin/vncm
/usr/bin/vnormalize
/usr/bin/vnormvals
/usr/bin/vnumave
/usr/bin/volumeinfo
/usr/bin/vonesample_ttest
/usr/bin/voverlap
/usr/bin/vpaired_ccm
/usr/bin/vpaired_ttest
/usr/bin/vpaired_wilcoxtest
/usr/bin/vpowerspectrum
/usr/bin/vpreprocess
/usr/bin/vpreproc_gui
/usr/bin/vpretty
/usr/bin/vreg3d
/usr/bin/vReHo
/usr/bin/vresiduals
/usr/bin/vrmglobal
/usr/bin/vROIonesample_ttest
/usr/bin/vROIpaired_ttest
/usr/bin/vROIpaired_wilcoxtest
/usr/bin/vROItwosample_ttest
/usr/bin/vseltimesteps
/usr/bin/vshowpts
/usr/bin/vsimmat
/usr/bin/vslicetime
/usr/bin/vspectral
/usr/bin/vspectralcluster
/usr/bin/vspectralecm
/usr/bin/vspm2lipsia
/usr/bin/vsrad
/usr/bin/vstats
/usr/bin/vsulcogram
/usr/bin/vswapdim
/usr/bin/vsymmetric
/usr/bin/vtal
/usr/bin/vtc
/usr/bin/vtimestep
/usr/bin/vtonifti
/usr/bin/vtrialaverage
/usr/bin/vtwosample_ttest
/usr/bin/vvinidi
/usr/bin/vwhiteglm
/usr/bin/vwhitematter
/usr/bin/vwilcoxon
/usr/bin/vzmapborder
/usr/bin/vzmax

/usr/include/gsl_utils.h

/usr/lib/lipsia/*

/usr/share/lipsia/*
/usr/share/doc/lipsia/*



%post

	chcon -t texrel_shlib_t /usr/lib/lipsia/* -R
