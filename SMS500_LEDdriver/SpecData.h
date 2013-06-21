#ifndef __SPECDATA_H__
#define __SPECDATA_H__

/*check if the compiler is of C++*/
#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
//	NAME:			DATE:			REMARKS:							 //
//  R. MOSCOSO		6/1/2007		Original release					 //
///////////////////////////////////////////////////////////////////////////

#define DLLEXPORT __declspec(dllexport)

#define MAX_SPEC		3					// Max. number of Channels
#define MAX_RANGES		23					// Max Number of Ranges for AVANTES and BWTEK
#define MAX_EX_RANGES	27					// Max Number of Ranges if max is 60000
#define	MAX_OO_RANGES	20					// Max Number of Ranges for OCEAN OPTICS
#define MAX_POINTS 		1000				// Max number of points
#define PIXEL_POINTS	2048				// CCD pixel points
#define AVA_PIX_LIMIT	2034				// PER AVASPEC AS-161 OEM MANUAL VER1.3 PG11-12

//Spectrometer types
#define BWTEK				3
#define AVANTES				0x0010
#define AVA_USB1			0x0011
#define LCSDATA				0
#define GONIODATA			1
// OceanOptics Spectrometers, defined in OOIDrv32.h, uncomment if not included
//#define ADC1000			0x0002 // ADC1000/PC2000
//#define USB2000			0x0004 // USB2000
//#define HR2000			0x0009 // HR2000

/**************************************************************************/
/* Error codes for DLL functions										  */
/**************************************************************************/
#define SPEC_NOT_FOUND			-20			//Spectrometer hardware not connected
#define SPEC_NOT_INIT			-21			//Spectrometer not initialized
#define SPEC_WL_ERR				-22			//start and stop wavelengths outside hardware limit
#define FILTER_NOT_FOUND		-23			//FilterOD*.cal file missing from app directory
#define DARK_NOT_FOUND			-24			//DarkRatioArray.dat missing from app directory
#define CAL_ERROR				-25			//Calibrated lamp file missing
#define DAT_ERROR				-26			//spectral data file not found or not loaded
#define AUX_ERROR				-27			//auxilliary lamp data file not found or not loaded
#define BWTEK_ERROR				-28			//undefined error from BWTEK spectrometer
#define NO_TRIGGER				-29			//external hardware trigger was not detected
//codes for Ocean Optics spectrometer errors
#define OO_NOERROR				0			// no error
#define OO_TIMEOUT				-2			// scan timed out
#define OO_BUSY					-3			// driver busy acquiring data
#define OO_SADSYNC				-4			// SAD500 synchronization error
#define OO_MEMORY				-5			// memory buffers not allocated
#define OO_UNKNOWN				-6			// unknown application instance
#define OO_TIMER				-7			// no timers available
#define OO_ERROR				-8			// unknown error
#define OO_NOTHREAD				-9			// unable to create Single Scan thread
#define OO_NIR_NOT_FOUND		-10			// unable to find NIR attached to system
#define OO_USB2000_NOT_FOUND 	-11			// unable to find USB2000 attached to system
#define OO_ADC1000USB_NOT_FOUND -12			// unable to find ADC1000-USB attached to system
#define OO_HR2000_NOT_FOUND		-13			// unable to find HR2000 attached to system

/**************************************************************************/
// COLORDATA structure for color data access
/**************************************************************************/
typedef struct Color_Par
{
    float 	X_relative;						// TRIS X value
    float	Y_relative;						// TRIS Y value
    float	Z_relative;						// TRIS Z value
    float	Xb_relative;					// TRIS Xb value
    float	x_value;						// CIE x value
    float	y_value;						// CIE y value
    float	z_value;						// CIE x value
    float	u_value;						// CIE u' (prime) value
    float	v_value;						// CIE v' (prime) value
    float	U_value;						// CIE u value
    float	V_value;						// CIE v value
    int		Color_val;						// Color  Temp. in Kelvins value
    float	Dominant;						// Domimant Wavelength
    float	Peak_wave;						// Peak Wavelength
    float 	Y_sum;							// Lumin value
}COLORDATA;


/**************************************************************************/
//RESULTDATA structure holds result of Spectral Scan
/**************************************************************************/
typedef struct Color_Data  {
    char	Oper[41];						// operator
    char	Cust[41];						// customer information
    char	Pnumb[41];						// part number information
    char	Commt[121];						// comment information
    char	Snumb[41];						// serial number information
    char	Dcode[41];						// holds Date code information
    char	TestTyp[41];					// Test Type information, pchModeType=Flux, ILEDA or ILEDB
    char	UnitTyp[41];					// Unit type information, pchPhotopicUnits=lumens or candela
    char	GraphUnit[41];					// Graph unit label
    char	PowerUnit[41];					// Power unit label
    char	Date[15];						// holds the date information
    char	Time[15];						// holds the time information
    float 	X_Master;						// holds the TRI X value Master Specrtometer
    float	Y_Master;						// holds the TRI Y value Master Specrtometer
    float	Z_Master;						// holds the TRI Z value Master Specrtometer
    float	XbMaster;						// holds the Xb relative value
    float	x_Master;						// holds the CIE x value Master Specrtometer
    float	y_Master;						// holds the CIE y value Master Specrtometer
    float	z_Master;						// holds the CIE z value data
    float	u_data;							// holds the u prime information
    float 	v_data;							// holds the v prime information
    float	U_data;							// holds the CIE u information
    float 	V_data;							// holds the CIE v information
    int 	Kelvin;							// holds the color temp. value
    double	Duv;							// distance from planckian locus on (u, v)
    float	Y_level;						// holds the scaled Y value CALCULATED NOT RAW
    float   Power;							// HOLDS THE SCANNED DATA POWER
    int 	WaveLength[MAX_POINTS];			// holds the Wave Length data
    double	MasterData[MAX_POINTS];			// holds the Master Data array
    double	PixelValues[MAX_POINTS];		// RAW PIXEL VALUES AT 1NM WAVELENGTHS
    int		Points;							// holds the number of data points to plot
    int		Start;							// holds the start wavelength value
    int		Stop;							// holds the stop wavelength value
    int		Inc;							// holds the Increment value
    int		Domwave;						// holds the Dominate wavelength value
    int		Peakwave;						// holds the Peak wavelength value
    double	IntgTime;						// holds the SPECTROMETER'S INTEGRATION TIME
    float	calFilterOD;					// CALIBRATION FILTER OPTICAL DENSITY 0, 0.5, 1.0, 2.0
    float	scanFilterOD;					// SCAN FILTER OPTICAL DENSITY 0, 0.5, 1.0, 2.0
    float   Purity;							// HOLDS THE PURITY VALUE
    float   R[14];							// STORES CALCULATED R VALUES
    float   Ra;								// STORES CALCULATED RA VALUE
    float	CriDC;
    int     CenterWL;						// HOLDS 1ST WL AT 50% PEAK/MAX VALUE
    int     FWHMWL;							// HOLDS 2ND WL AT 50% PEAK/MAX VALUE
    int 	CentroidWL;						// HOLDS THE CENTROID WAVELENGTH VALUE
    double	CDdistance;						// needed for intensity calculations
    double	aperDiameter;					// needed for intensity calculations
}RESULTDATA;


/**************************************************************************/
// SCANSPEC structure for color data access
/**************************************************************************/
typedef struct tagSPTRChannel
{
    short	Master;							// 1 - Active, 0 - Disabled
    int		SpectType;						// BWTEK=3, AVANTES=0x0010, AVA_USB1=0x0011
    char	SerialNum[30];					// Spectrometer's serial number, Model number for BWTEK
    short	Status;							// Spectrometer Status flag
    short	AutoRang;						// Spectrometer Autorange flag
    int		Range;							// Spectrometer range setting
    double	Cof1;							// Spec. Coefficient #1
    double	Cof2;							// Spec. Coefficient #2
    double	Cof3;							// Spec. Coefficient #3
    double	LinCof[8];						// Spec. 7th-order detector linearity coefficient
    double	Intercept;						// Spec. Intercept
    int		IsectPoint;						// WL where dual Avantes intersect
    int		DarkPixelStart;					// Spec. Start BASE0 DARK PIXEL POINT
    int		DarkPixelStop;					// Spec. Stop BASE0 DARK PIXEL POINT
    int		StartWave;						// Spec. Start Wavelength
    int		StopWave;						// Spec. Stop Wavelength
    double	IntTime;						// Spectrometer integration time
    int		MinIntTime;						// Minimum Integration for BWTEK
    int		Trigger;						// SPEC INTERNAL=0 EXTERNAL=1 TRIGGER MODE
    int 	SoakTime;						// Soak time(ms), used for timeout of external trigger
    int		NumTestPoints;					// Holds the number of spectrometer test points
    int		Saturated;						// TRUE or FALSE for saturated spectrometer
    int 	MaxSpCounts;					// Maximun Spectrometer Counts
    int 	SetMaxCounts;					// Max count limits from *.ini file
    int 	SetMinCounts;					// Min count limits from *.ini file
    short	BoxCar;							// Spectrometer Box Car Soothing
    short	Aveg;							// Number of points to average
    short	CorrDark;						// Correct for dark current
    float	calFilterOD;					// CALIBRATION FILTER OPTICAL DENSITY 0, 0.5, 1.0, 2.0
    float	scanFilterOD;					// SCAN FILTER OPTICAL DENSITY 0, 0.5, 1.0, 2.0
    float	DarkPixelAvg;					// P15-P25 MASKED DARK PIXELS AVERAGE
    float	GonioCalFactor;					// GONIO CALIBRATION DATA FACTOR
    double	Data[MAX_POINTS];				// Spectrometer Data
    double	PixelValues[MAX_POINTS];		// RAW PIXEL VALUES AT 1NM WAVELENGTHS
    float	WaveLength[MAX_POINTS];			// Spectrometer Wavelength
    char	*pchModeType;					// Flux, ILEDA, ILEDB, SCP, Lumens, lux, fc, Intensity, cd/m2, fL
    char    *pchOpMode;						// Either "GONIO" or "LCS"
} SPTRChannel;

typedef struct tagSpectrometer
{
    int         nChannels;					// Spectrometer Channel 0, 1, 2 only
    SPTRChannel Channel[MAX_SPEC];

} SPECTROMETER;

// Integration time for Avantes USB2
const double INTEGRATION_TIME[MAX_EX_RANGES] = {1.1, 2, 3, 4, 5, 6, 8, 10, 15, 25,35, 45, 65,100, 130, 190, 260,
                                               375, 500, 750, 1000, 2000, 4000, 8000, 16000, 32000, 60000};

//EXPORT FUNCTIONS
int 	DLLEXPORT ReadCalStd(char *calFile);
int 	DLLEXPORT ReadGonioCalStd(char *calFile);
int		DLLEXPORT ReadSpecDataFile(char *sSpecDataFile);
int		DLLEXPORT ReadAuxCorrFile(char *sAuxCorrFile);
double 	DLLEXPORT GetSpecDLLVersion(void);
void	DLLEXPORT SetLinearityCoefs(double Coefs[8], int channelNum);
int 	DLLEXPORT FindInitSpectrometer(SPECTROMETER* ptr);
int		DLLEXPORT FindInitBWTekSpectrometer(SPECTROMETER *ptr);
int		DLLEXPORT CloseSpectrometer(void);
int 	DLLEXPORT UpdateCalData(SPECTROMETER *ptr, char *datFile);
int		DLLEXPORT StartLampScan(SPECTROMETER *ptr, int auxScan);
int		DLLEXPORT FinishLampScan(SPECTROMETER *ptr, char *datFile, int auxScan);
int 	DLLEXPORT GetSpectralData (SPECTROMETER *spec_ptr, RESULTDATA *data_ptr, int auxCorr);
int 	DLLEXPORT ReadDarkCurrent( SPECTROMETER *ptr);
int		DLLEXPORT CreateDarkRatioArray(SPECTROMETER *ptr);
double	DLLEXPORT HEC_CalculateXYValue(double *Wave_val, COLORDATA *cp, int iStartWave, int iStopWave);
int		DLLEXPORT EnableStrayLightCorrection(int enable, char *sSLCorrFile, double factor);
void	DLLEXPORT EnableNoiseReduction(int enable, double factor);
void	DLLEXPORT SetPixelAddition(int numAdjPixels, double pixelFactor, int channelNum);
void	DLLEXPORT ExtendIntTimeRange(int enable);
void	DLLEXPORT SpecDataDLLDebug(int enable);

/*check if the compiler is of C++*/
#ifdef __cplusplus
}
#endif

#endif // __SPECDATA_H__
