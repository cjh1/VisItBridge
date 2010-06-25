PROJECT(VisItNETCDF)

SET(SOURCES
avtBasicNETCDFFileFormat.C
avtADAPTFileFormat.C
avtLODIFileFormat.C
avtLODIParticleFileFormat.C
avtFVCOM_STSDFileFormat.C
avtFVCOMParticleFileFormat.C
avtFVCOM_MTSDFileFormat.C
avtFVCOM_MTMDFileFormat.C
avtCCSMFileFormat.C
avtNETCDFReaderBase.C
avtBasicNETCDFReader.C
avtCCSMReader.C
avtFVCOMReader.C
NETCDFFileObject.C
)

#needed by cmake to properly generate the correct readers in the interface collection
#for each reader in the interface we need: name, type, and name of header it is defined in
set(READER_NAMES
avtADAPTFileFormat
avtLODIFileFormat
avtLODIParticleFileFormat
avtFVCOM_STSDFileFormat
avtFVCOM_MTMDFileFormat
avtFVCOM_MTSDFileFormat
avtFVCOMParticleFileFormat
avtCCSM_MTSD_FileFormat
avtCCSM_STSD_FileFormat
avtBasic_MTSD_NETCDFFileFormat
avtBasic_STSD_NETCDFFileFormat
)
set(READER_TYPES
STSD
MTSD
MTSD
STSD
MTMD
MTSD
MTSD
MTSD
STSD
MTSD
STSD
)

set(READER_INCLUDES
avtADAPTFileFormat
avtLODIFileFormat
avtLODIParticleFileFormat
avtFVCOM_STSDFileFormat
avtFVCOM_MTMDFileFormat
avtFVCOM_MTSDFileFormat
avtFVCOMParticleFileFormat
avtCCSMFileFormat
avtCCSMFileFormat
avtBasicNETCDFFileFormat
avtBasicNETCDFFileFormat
)

ADD_VISIT_INTERFACE_READER(VisItNETCDF "1.0"    
  VISIT_READER_NAMES ${READER_NAMES}        
  VISIT_READER_TYPES ${READER_TYPES} 
  VISIT_READER_INCLUDES ${READER_INCLUDES}
  VISIT_READER_FILE_PATTERN "cdf nc elev ncd"
  SERVER_SOURCES ${SOURCES}  
  )

TARGET_LINK_LIBRARIES(VisItNETCDF AVTAlgorithms)