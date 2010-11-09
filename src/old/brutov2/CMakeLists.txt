PROJECT(BRUTOV2)

ADD_EXECUTABLE(brutov2
    BrukerPV11Reader.C DoubleOption.C IOption.C  Logger.C StringListValidator.C
    BrukerPV11ToVistaConverter.C BrukerPV11ToNiftiConverter.C
    DoubleRangeValidator.C Image.C  OptionSet.C StringOption.C BrukerPV3Reader.C
    EmptyConverter.C ImageDatasetParameter.C OptionValues.C
    StringRegexpValidator.C BrukerPV3ToVistaConverter.C
    BrukerPV3ToNiftiConverter.C FileOption.C  IntegerOption.C Parameter.C
    VistaWriter.C NiftiWriter.C ConverterConfiguration.C FileRegexpValidator.C
    IntegerRangeValidator.C  RequiredValidator.C  brutov2.C ConverterFactory.C
    FileTypeValidator.C JCampDXParser.C StringListOption.C)

TARGET_LINK_LIBRARIES(brutov2 ${VIAIO_LIBRARY})

SET_TARGET_PROPERTIES(brutov2
    PROPERTIES COMPILE_FLAGS -ansi
    LINK_FLAGS -Wl)

INSTALL(TARGETS brutov2
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
