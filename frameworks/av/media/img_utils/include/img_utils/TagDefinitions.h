/*
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IMG_UTILS_TIFF_TAG_DEFINITION_H
#define IMG_UTILS_TIFF_TAG_DEFINITION_H

#include <img_utils/TiffEntry.h>
#include <img_utils/Output.h>
#include <img_utils/TiffHelpers.h>
#include <stdint.h>

namespace android {
namespace img_utils {

/**
 * Tag definitions contain information about standard TIFF compatible tags.
 */
typedef struct TagDefinition {
    // The tag name.
    const char* tagName;
    // The specified tag ID.
    const uint16_t tagId;
    // The default type for this tag.  This must be a valid TIFF type.
    const TagType defaultType;
    // The default Image File Directory (IFD) for this tag.
    const uint32_t defaultIfd;
    // The valid count for this tag, or 0 if the count is not fixed.
    const uint32_t fixedCount;
    // The endianness of the tag value, or UNDEFINED_ENDIAN if there is no fixed endian
    const Endianness fixedEndian;
} TagDefinition_t;

/**
 * Convenience defines for tag ids.
 */
enum {
    TAG_RAWTOPREVIEWGAIN = 0xC7A8u,
    TAG_NEWRAWIMAGEDIGEST = 0xC7A7u,
    TAG_ORIGINALDEFAULTCROPSIZE = 0xC793u,
    TAG_ORIGINALBESTQUALITYFINALSIZE = 0xC792u,
    TAG_ORIGINALDEFAULTFINALSIZE = 0xC791u,
    TAG_PROFILEHUESATMAPENCODING = 0xC7A3u,
    TAG_PROFILELOOKTABLEENCODING = 0xC7A4u,
    TAG_BASELINEEXPOSUREOFFSET = 0xC7A5u,
    TAG_DEFAULTBLACKRENDER = 0xC7A6u,
    TAG_DEFAULTUSERCROP = 0xC7B5u,
    TAG_NOISEPROFILE = 0xC761u,
    TAG_OPCODELIST3 = 0xC74Eu,
    TAG_OPCODELIST2 = 0xC741u,
    TAG_OPCODELIST1 = 0xC740u,
    TAG_PROFILELOOKTABLEDATA = 0xC726u,
    TAG_PROFILELOOKTABLEDIMS = 0xC725u,
    TAG_ROWINTERLEAVEFACTOR = 0xC71Fu,
    TAG_SUBTILEBLOCKSIZE = 0xC71Eu,
    TAG_ORIGINALRAWFILEDIGEST = 0xC71Du,
    TAG_RAWIMAGEDIGEST = 0xC71Cu,
    TAG_PREVIEWDATETIME = 0xC71Bu,
    TAG_PREVIEWCOLORSPACE = 0xC71Au,
    TAG_PREVIEWSETTINGSDIGEST = 0xC719u,
    TAG_PREVIEWSETTINGSNAME = 0xC718u,
    TAG_PREVIEWAPPLICATIONVERSION = 0xC717u,
    TAG_PREVIEWAPPLICATIONNAME = 0xC716u,
    TAG_FORWARDMATRIX2 = 0xC715u,
    TAG_FORWARDMATRIX1 = 0xC714u,
    TAG_PROFILECOPYRIGHT = 0xC6FEu,
    TAG_PROFILEEMBEDPOLICY = 0xC6FDu,
    TAG_PROFILETONECURVE = 0xC6FCu,
    TAG_PROFILEHUESATMAPDATA2 = 0xC6FBu,
    TAG_PROFILEHUESATMAPDATA1 = 0xC6FAu,
    TAG_PROFILEHUESATMAPDIMS = 0xC6F9u,
    TAG_PROFILENAME = 0xC6F8u,
    TAG_NOISEREDUCTIONAPPLIED = 0xC6F7u,
    TAG_ASSHOTPROFILENAME = 0xC6F6u,
    TAG_EXTRACAMERAPROFILES = 0xC6F5u,
    TAG_PROFILECALIBRATIONSIGNATURE = 0xC6F4u,
    TAG_CAMERACALIBRATIONSIGNATURE = 0xC6F3u,
    TAG_COLORIMETRICREFERENCE = 0xC6BFu,
    TAG_CURRENTPREPROFILEMATRIX = 0xC692u,
    TAG_CURRENTICCPROFILE = 0xC691u,
    TAG_ASSHOTPREPROFILEMATRIX = 0xC690u,
    TAG_ASSHOTICCPROFILE = 0xC68Fu,
    TAG_MASKEDAREAS = 0xC68Eu,
    TAG_ACTIVEAREA = 0xC68Du,
    TAG_ORIGINALRAWFILEDATA = 0xC68Cu,
    TAG_ORIGINALRAWFILENAME = 0xC68Bu,
    TAG_RAWDATAUNIQUEID = 0xC65Du,
    TAG_MAKERNOTESAFETY = 0xC635u,
    TAG_DNGPRIVATEDATA = 0xC634u,
    TAG_SHADOWSCALE = 0xC633u,
    TAG_ANTIALIASSTRENGTH = 0xC632u,
    TAG_CHROMABLURRADIUS = 0xC631u,
    TAG_LENSINFO = 0xC630u,
    TAG_CAMERASERIALNUMBER = 0xC62Fu,
    TAG_LINEARRESPONSELIMIT = 0xC62Eu,
    TAG_BAYERGREENSPLIT = 0xC62Du,
    TAG_BASELINESHARPNESS = 0xC62Cu,
    TAG_BASELINENOISE = 0xC62Bu,
    TAG_BASELINEEXPOSURE = 0xC62Au,
    TAG_ASSHOTWHITEXY = 0xC629u,
    TAG_ASSHOTNEUTRAL = 0xC628u,
    TAG_ANALOGBALANCE = 0xC627u,
    TAG_REDUCTIONMATRIX2 = 0xC626u,
    TAG_REDUCTIONMATRIX1 = 0xC625u,
    TAG_CAMERACALIBRATION2 = 0xC624u,
    TAG_CAMERACALIBRATION1 = 0xC623u,
    TAG_COLORMATRIX2 = 0xC622u,
    TAG_COLORMATRIX1 = 0xC621u,
    TAG_CALIBRATIONILLUMINANT2 = 0xC65Bu,
    TAG_CALIBRATIONILLUMINANT1 = 0xC65Au,
    TAG_DEFAULTCROPSIZE = 0xC620u,
    TAG_DEFAULTCROPORIGIN = 0xC61Fu,
    TAG_BESTQUALITYSCALE = 0xC65Cu,
    TAG_DEFAULTSCALE = 0xC61Eu,
    TAG_WHITELEVEL = 0xC61Du,
    TAG_BLACKLEVELDELTAV = 0xC61Cu,
    TAG_BLACKLEVELDELTAH = 0xC61Bu,
    TAG_BLACKLEVEL = 0xC61Au,
    TAG_BLACKLEVELREPEATDIM = 0xC619u,
    TAG_LINEARIZATIONTABLE = 0xC618u,
    TAG_CFALAYOUT = 0xC617u,
    TAG_CFAPLANECOLOR = 0xC616u,
    TAG_LOCALIZEDCAMERAMODEL = 0xC615u,
    TAG_UNIQUECAMERAMODEL = 0xC614u,
    TAG_DNGBACKWARDVERSION = 0xC613u,
    TAG_DNGVERSION = 0xC612u,
    TAG_SUBFILETYPE = 0x00FFu,
    TAG_YRESOLUTION = 0x011Bu,
    TAG_XRESOLUTION = 0x011Au,
    TAG_THRESHHOLDING = 0x0107u,
    TAG_STRIPOFFSETS = 0x0111u,
    TAG_STRIPBYTECOUNTS = 0x0117u,
    TAG_SOFTWARE = 0x0131u,
    TAG_SAMPLESPERPIXEL = 0x0115u,
    TAG_ROWSPERSTRIP = 0x0116u,
    TAG_RESOLUTIONUNIT = 0x0128u,
    TAG_PLANARCONFIGURATION = 0x011Cu,
    TAG_PHOTOMETRICINTERPRETATION = 0x0106u,
    TAG_ORIENTATION = 0x0112u,
    TAG_NEWSUBFILETYPE = 0x00FEu,
    TAG_MODEL = 0x0110u,
    TAG_MINSAMPLEVALUE = 0x0118u,
    TAG_MAXSAMPLEVALUE = 0x0119u,
    TAG_MAKE = 0x010Fu,
    TAG_IMAGEWIDTH = 0x0100u,
    TAG_IMAGELENGTH = 0x0101u,
    TAG_IMAGEDESCRIPTION = 0x010Eu,
    TAG_HOSTCOMPUTER = 0x013Cu,
    TAG_GRAYRESPONSEUNIT = 0x0122u,
    TAG_GRAYRESPONSECURVE = 0x0123u,
    TAG_FREEOFFSETS = 0x0120u,
    TAG_FREEBYTECOUNTS = 0x0121u,
    TAG_FILLORDER = 0x010Au,
    TAG_EXTRASAMPLES = 0x0152u,
    TAG_DATETIME = 0x0132u,
    TAG_COPYRIGHT = 0x8298u,
    TAG_COMPRESSION = 0x0103u,
    TAG_COLORMAP = 0x0140u,
    TAG_CELLWIDTH = 0x0108u,
    TAG_CELLLENGTH = 0x0109u,
    TAG_BITSPERSAMPLE = 0x0102u,
    TAG_ARTIST = 0x013Bu,
    TAG_EXIFVERSION = 0x9000u,
    TAG_CFAREPEATPATTERNDIM = 0x828Du,
    TAG_DATETIMEORIGINAL = 0x9003u,
    TAG_CFAPATTERN = 0x828Eu,
    TAG_SUBIFDS = 0x014Au,
    TAG_TIFFEPSTANDARDID = 0x9216u,
    TAG_EXPOSURETIME = 0x829Au,
    TAG_ISOSPEEDRATINGS = 0x8827u,
    TAG_FOCALLENGTH = 0x920Au,
    TAG_FNUMBER = 0x829Du,
    TAG_GPSINFO = 0x8825u,
    TAG_GPSVERSIONID = 0x0u,
    TAG_GPSLATITUDEREF = 0x1u,
    TAG_GPSLATITUDE = 0x2u,
    TAG_GPSLONGITUDEREF = 0x3u,
    TAG_GPSLONGITUDE = 0x4u,
    TAG_GPSTIMESTAMP = 0x7u,
    TAG_GPSDATESTAMP = 0x001Du,
};

/**
 * Convenience values for tags with enumerated values
 */

enum {
    TAG_ORIENTATION_NORMAL = 1,
    TAG_ORIENTATION_ROTATE_180 = 3,
    TAG_ORIENTATION_ROTATE_90 = 6,
    TAG_ORIENTATION_ROTATE_270 = 8,
    TAG_ORIENTATION_UNKNOWN = 9
};

/**
 * TIFF_EP_TAG_DEFINITIONS contains tags defined in the TIFF EP spec
 */
const TagDefinition_t TIFF_EP_TAG_DEFINITIONS[] =  {
    { // PhotometricInterpretation
        "PhotometricInterpretation",
        0x0106u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // SubIfds
        "SubIfds",
        0x014Au,
        LONG,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CFAPattern
        "CFAPattern",
        0x828Eu,
        BYTE,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CFARepeatPatternDim
        "CFARepeatPatternDim",
        0x828Du,
        SHORT,
        IFD_0,
        2,
        UNDEFINED_ENDIAN
    },
    { // DateTimeOriginal
        "DateTimeOriginal",
        0x9003u,
        ASCII,
        IFD_0,
        20,
        UNDEFINED_ENDIAN
    },
    { // Tiff/EPStandardID
        "Tiff",
        0x9216u,
        BYTE,
        IFD_0,
        4,
        UNDEFINED_ENDIAN
    },
    { // ExposureTime
        "ExposureTime",
        0x829Au,
        RATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // ISOSpeedRatings
        "ISOSpeedRatings",
        0x8827u,
        SHORT,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // FocalLength
        "FocalLength",
        0x920Au,
        RATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // FNumber
        "FNumber",
        0x829Du,
        RATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // GPSInfo
        "GPSInfo",
        0x8825u,
        LONG,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // GPSVersionID
        "GPSVersionID",
        0x0u,
        BYTE,
        IFD_0,
        4,
        UNDEFINED_ENDIAN
    },
    { // GPSLatitudeRef
        "GPSLatitudeRef",
        0x1u,
        ASCII,
        IFD_0,
        2,
        UNDEFINED_ENDIAN
    },
    { // GPSLatitude
        "GPSLatitude",
        0x2u,
        RATIONAL,
        IFD_0,
        3,
        UNDEFINED_ENDIAN
    },
    { // GPSLongitudeRef
        "GPSLongitudeRef",
        0x3u,
        ASCII,
        IFD_0,
        2,
        UNDEFINED_ENDIAN
    },
    { // GPSLongitude
        "GPSLongitude",
        0x4u,
        RATIONAL,
        IFD_0,
        3,
        UNDEFINED_ENDIAN
    },
    { // GPSTimeStamp
        "GPSTimeStamp",
        0x7u,
        RATIONAL,
        IFD_0,
        3,
        UNDEFINED_ENDIAN
    },
    /*TODO: Remaining TIFF EP tags*/
};

/**
 * EXIF_2_3_TAG_DEFINITIONS contains tags defined in the Jeita EXIF 2.3 spec
 */
const TagDefinition_t EXIF_2_3_TAG_DEFINITIONS[] = {
    { // ExifVersion
        "ExifVersion",
        0x9000u,
        UNDEFINED,
        IFD_0,
        4,
        UNDEFINED_ENDIAN
    },
    { // GPSDateStamp
        "GPSDateStamp",
        0x001Du,
        ASCII,
        IFD_0,
        11,
        UNDEFINED_ENDIAN
    },
    /*TODO: Remaining EXIF 2.3 tags*/
};

/**
 * TIFF_6_TAG_DEFINITIONS contains tags defined in the TIFF 6.0 spec
 */
const TagDefinition_t TIFF_6_TAG_DEFINITIONS[] = {
    { // SubFileType
        "SubFileType",
        0x00FFu,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // Artist
        "Artist",
        0x013Bu,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // BitsPerSample
        "BitsPerSample",
        0x0102u,
        SHORT,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CellLength
        "CellLength",
        0x0109u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // CellWidth
        "CellWidth",
        0x0108u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // ColorMap
        "ColorMap",
        0x0140u,
        SHORT,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // Compression
        "Compression",
        0x0103u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // Copyright
        "Copyright",
        0x8298u,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // DateTime
        "DateTime",
        0x0132u,
        ASCII,
        IFD_0,
        20,
        UNDEFINED_ENDIAN
    },
    { // ExtraSamples
        "ExtraSamples",
        0x0152u,
        SHORT,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // FillOrder
        "FillOrder",
        0x010Au,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // FreeByteCounts
        "FreeByteCounts",
        0x0121u,
        LONG,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // FreeOffsets
        "FreeOffsets",
        0x0120u,
        LONG,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // GrayResponseCurve
        "GrayResponseCurve",
        0x0123u,
        SHORT,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // GrayResponseUnit
        "GrayResponseUnit",
        0x0122u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // HostComputer
        "HostComputer",
        0x013Cu,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // ImageDescription
        "ImageDescription",
        0x010Eu,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // ImageLength
        "ImageLength",
        0x0101u,
        LONG,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // ImageWidth
        "ImageWidth",
        0x0100u,
        LONG,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // Make
        "Make",
        0x010Fu,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // MaxSampleValue
        "MaxSampleValue",
        0x0119u,
        SHORT,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // MinSampleValue
        "MinSampleValue",
        0x0118u,
        SHORT,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // Model
        "Model",
        0x0110u,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // NewSubfileType
        "NewSubfileType",
        0x00FEu,
        LONG,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // Orientation
        "Orientation",
        0x0112u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // PhotoMetricInterpretation
        "PhotoMetricInterpretation",
        0x0106u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // PlanarConfiguration
        "PlanarConfiguration",
        0x011Cu,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // ResolutionUnit
        "ResolutionUnit",
        0x0128u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // RowsPerStrip
        "RowsPerStrip",
        0x0116u,
        LONG,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // SamplesPerPixel
        "SamplesPerPixel",
        0x0115u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // Software
        "Software",
        0x0131u,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // StripByteCounts
        "StripByteCounts",
        0x0117u,
        LONG,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // StripOffsets
        "StripOffsets",
        0x0111u,
        LONG,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // SubfileType
        "SubfileType",
        0x00FFu,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // Threshholding
        "Threshholding",
        0x0107u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // XResolution
        "XResolution",
        0x011Au,
        RATIONAL,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // YResolution
        "YResolution",
        0x011Bu,
        RATIONAL,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
};

/**
 * DNG_TAG_DEFINITIONS contains tags defined in the DNG 1.4 spec
 */
const TagDefinition_t DNG_TAG_DEFINITIONS[] = {
    { // DNGVersion
        "DNGVersion",
        0xC612u,
        BYTE,
        IFD_0,
        4,
        UNDEFINED_ENDIAN
    },
    { // DNGBackwardVersion
        "DNGBackwardVersion",
        0xC613u,
        BYTE,
        IFD_0,
        4,
        UNDEFINED_ENDIAN
    },
    { // UniqueCameraModel
        "UniqueCameraModel",
        0xC614u,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // LocalizedCameraModel
        "LocalizedCameraModel",
        0xC615u,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CFAPlaneColor
        "CFAPlaneColor",
        0xC616u,
        BYTE,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // CFALayout
        "CFALayout",
        0xC617u,
        SHORT,
        RAW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // LinearizationTable
        "LinearizationTable",
        0xC618u,
        SHORT,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // BlackLevelRepeatDim
        "BlackLevelRepeatDim",
        0xC619u,
        SHORT,
        RAW_IFD,
        2,
        UNDEFINED_ENDIAN
    },
    { // BlackLevel
        "BlackLevel",
        0xC61Au,
        RATIONAL,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // BlackLevelDeltaH
        "BlackLevelDeltaH",
        0xC61Bu,
        SRATIONAL,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // BlackLevelDeltaV
        "BlackLevelDeltaV",
        0xC61Cu,
        SRATIONAL,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // WhiteLevel
        "WhiteLevel",
        0xC61Du,
        LONG,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // DefaultScale
        "DefaultScale",
        0xC61Eu,
        RATIONAL,
        RAW_IFD,
        2,
        UNDEFINED_ENDIAN
    },
    { // BestQualityScale
        "BestQualityScale",
        0xC65Cu,
        RATIONAL,
        RAW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // DefaultCropOrigin
        "DefaultCropOrigin",
        0xC61Fu,
        LONG,
        RAW_IFD,
        2,
        UNDEFINED_ENDIAN
    },
    { // DefaultCropSize
        "DefaultCropSize",
        0xC620u,
        LONG,
        RAW_IFD,
        2,
        UNDEFINED_ENDIAN
    },
    { // CalibrationIlluminant1
        "CalibrationIlluminant1",
        0xC65Au,
        SHORT,
        PROFILE_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // CalibrationIlluminant2
        "CalibrationIlluminant2",
        0xC65Bu,
        SHORT,
        PROFILE_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // ColorMatrix1
        "ColorMatrix1",
        0xC621u,
        SRATIONAL,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ColorMatrix2
        "ColorMatrix2",
        0xC622u,
        SRATIONAL,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // CameraCalibration1
        "CameraCalibration1",
        0xC623u,
        SRATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CameraCalibration2
        "CameraCalibration2",
        0xC624u,
        SRATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // ReductionMatrix1
        "ReductionMatrix1",
        0xC625u,
        SRATIONAL,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ReductionMatrix2
        "ReductionMatrix2",
        0xC626u,
        SRATIONAL,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // AnalogBalance
        "AnalogBalance",
        0xC627u,
        RATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // AsShotNeutral
        "AsShotNeutral",
        0xC628u,
        RATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // AsShotWhiteXY
        "AsShotWhiteXY",
        0xC629u,
        RATIONAL,
        IFD_0,
        2,
        UNDEFINED_ENDIAN
    },
    { // BaselineExposure
        "BaselineExposure",
        0xC62Au,
        SRATIONAL,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // BaselineNoise
        "BaselineNoise",
        0xC62Bu,
        RATIONAL,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // BaselineSharpness
        "BaselineSharpness",
        0xC62Cu,
        RATIONAL,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // BayerGreenSplit
        "BayerGreenSplit",
        0xC62Du,
        LONG,
        RAW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // LinearResponseLimit
        "LinearResponseLimit",
        0xC62Eu,
        RATIONAL,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // CameraSerialNumber
        "CameraSerialNumber",
        0xC62Fu,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // LensInfo
        "LensInfo",
        0xC630u,
        RATIONAL,
        IFD_0,
        4,
        UNDEFINED_ENDIAN
    },
    { // ChromaBlurRadius
        "ChromaBlurRadius",
        0xC631u,
        RATIONAL,
        RAW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // AntiAliasStrength
        "AntiAliasStrength",
        0xC632u,
        RATIONAL,
        RAW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // ShadowScale
        "ShadowScale",
        0xC633u,
        RATIONAL,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // DNGPrivateData
        "DNGPrivateData",
        0xC634u,
        BYTE,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // MakerNoteSafety
        "MakerNoteSafety",
        0xC635u,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // RawDataUniqueID
        "RawDataUniqueID",
        0xC65Du,
        BYTE,
        IFD_0,
        16,
        UNDEFINED_ENDIAN
    },
    { // OriginalRawFileName
        "OriginalRawFileName",
        0xC68Bu,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // OriginalRawFileData
        "OriginalRawFileData",
        0xC68Cu,
        UNDEFINED,
        IFD_0,
        0,
        BIG
    },
    { // ActiveArea
        "ActiveArea",
        0xC68Du,
        LONG,
        RAW_IFD,
        4,
        UNDEFINED_ENDIAN
    },
    { // MaskedAreas
        "MaskedAreas",
        0xC68Eu,
        LONG,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // AsShotICCProfile
        "AsShotICCProfile",
        0xC68Fu,
        UNDEFINED,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // AsShotPreProfileMatrix
        "AsShotPreProfileMatrix",
        0xC690u,
        SRATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CurrentICCProfile
        "CurrentICCProfile",
        0xC691u,
        UNDEFINED,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CurrentICCProfile
        "CurrentICCProfile",
        0xC691u,
        UNDEFINED,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // CurrentPreProfileMatrix
        "CurrentPreProfileMatrix",
        0xC692u,
        SRATIONAL,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // ColorimetricReference
        "ColorimetricReference",
        0xC6BFu,
        SHORT,
        IFD_0,
        1,
        UNDEFINED_ENDIAN
    },
    { // CameraCalibrationSignature
        "CameraCalibrationSignature",
        0xC6F3u,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // ProfileCalibrationSignature
        "ProfileCalibrationSignature",
        0xC6F4u,
        ASCII,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ExtraCameraProfiles
        "ExtraCameraProfiles",
        0xC6F5u,
        LONG,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // AsShotProfileName
        "AsShotProfileName",
        0xC6F6u,
        ASCII,
        IFD_0,
        0,
        UNDEFINED_ENDIAN
    },
    { // NoiseReductionApplied
        "NoiseReductionApplied",
        0xC6F7u,
        RATIONAL,
        RAW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // ProfileName
        "ProfileName",
        0xC6F8u,
        ASCII,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ProfileHueSatMapDims
        "ProfileHueSatMapDims",
        0xC6F9u,
        LONG,
        PROFILE_IFD,
        3,
        UNDEFINED_ENDIAN
    },
    { // ProfileHueSatMapData1
        "ProfileHueSatMapData1",
        0xC6FAu,
        FLOAT,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ProfileHueSatMapData2
        "ProfileHueSatMapData2",
        0xC6FBu,
        FLOAT,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ProfileToneCurve
        "ProfileToneCurve",
        0xC6FCu,
        FLOAT,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ProfileEmbedPolicy
        "ProfileEmbedPolicy",
        0xC6FDu,
        LONG,
        PROFILE_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // ProfileCopyright
        "ProfileCopyright",
        0xC6FEu,
        ASCII,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ForwardMatrix1
        "ForwardMatrix1",
        0xC714u,
        SRATIONAL,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // ForwardMatrix2
        "ForwardMatrix2",
        0xC715u,
        SRATIONAL,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // PreviewApplicationName
        "PreviewApplicationName",
        0xC716u,
        ASCII,
        PREVIEW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // PreviewApplicationVersion
        "PreviewApplicationVersion",
        0xC717u,
        ASCII,
        PREVIEW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // PreviewSettingsName
        "PreviewSettingsName",
        0xC718u,
        ASCII,
        PREVIEW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // PreviewSettingsDigest
        "PreviewSettingsDigest",
        0xC719u,
        BYTE,
        PREVIEW_IFD,
        16,
        UNDEFINED_ENDIAN
    },
    { // PreviewColorSpace
        "PreviewColorSpace",
        0xC71Au,
        LONG,
        PREVIEW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // PreviewDateTime
        "PreviewDateTime",
        0xC71Bu,
        ASCII,
        PREVIEW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // RawImageDigest
        "RawImageDigest",
        0xC71Cu,
        BYTE,
        IFD_0,
        16,
        UNDEFINED_ENDIAN
    },
    { // OriginalRawFileDigest
        "OriginalRawFileDigest",
        0xC71Du,
        BYTE,
        IFD_0,
        16,
        UNDEFINED_ENDIAN
    },
    { // SubTileBlockSize
        "SubTileBlockSize",
        0xC71Eu,
        LONG,
        RAW_IFD,
        2,
        UNDEFINED_ENDIAN
    },
    { // RowInterleaveFactor
        "RowInterleaveFactor",
        0xC71Fu,
        LONG,
        RAW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // ProfileLookTableDims
        "ProfileLookTableDims",
        0xC725u,
        LONG,
        PROFILE_IFD,
        3,
        UNDEFINED_ENDIAN
    },
    { // ProfileLookTableData
        "ProfileLookTableData",
        0xC726u,
        FLOAT,
        PROFILE_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // OpcodeList1
        "OpcodeList1",
        0xC740u,
        UNDEFINED,
        RAW_IFD,
        0,
        BIG
    },
    { // OpcodeList2
        "OpcodeList2",
        0xC741u,
        UNDEFINED,
        RAW_IFD,
        0,
        BIG
    },
    { // OpcodeList3
        "OpcodeList3",
        0xC74Eu,
        UNDEFINED,
        RAW_IFD,
        0,
        BIG
    },
    { // NoiseProfile
        "NoiseProfile",
        0xC761u,
        DOUBLE,
        RAW_IFD,
        0,
        UNDEFINED_ENDIAN
    },
    { // DefaultUserCrop
        "DefaultUserCrop",
        0xC7B5u,
        RATIONAL,
        RAW_IFD,
        4,
        UNDEFINED_ENDIAN
    },
    { // DefaultBlackRender
        "DefaultBlackRender",
        0xC7A6u,
        LONG,
        PROFILE_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // BaselineExposureOffset
        "BaselineExposureOffset",
        0xC7A5u,
        RATIONAL,
        PROFILE_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // ProfileLookTableEncoding
        "ProfileLookTableEncoding",
        0xC7A4u,
        LONG,
        PROFILE_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // ProfileHueSatMapEncoding
        "ProfileHueSatMapEncoding",
        0xC7A3u,
        LONG,
        PROFILE_IFD,
        1,
        UNDEFINED_ENDIAN
    },
    { // OriginalDefaultFinalSize
        "OriginalDefaultFinalSize",
        0xC791u,
        LONG,
        IFD_0,
        2,
        UNDEFINED_ENDIAN
    },
    { // OriginalBestQualityFinalSize
        "OriginalBestQualityFinalSize",
        0xC792u,
        LONG,
        IFD_0,
        2,
        UNDEFINED_ENDIAN
    },
    { // OriginalDefaultCropSize
        "OriginalDefaultCropSize",
        0xC793u,
        LONG,
        IFD_0,
        2,
        UNDEFINED_ENDIAN
    },
    { // NewRawImageDigest
        "NewRawImageDigest",
        0xC7A7u,
        BYTE,
        IFD_0,
        16,
        UNDEFINED_ENDIAN
    },
    { // RawToPreviewGain
        "RawToPreviewGain",
        0xC7A8u,
        DOUBLE,
        PREVIEW_IFD,
        1,
        UNDEFINED_ENDIAN
    },
};

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_TIFF_TAG_DEFINITION_H*/
