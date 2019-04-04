#include "stdafx.h"
#include <math.h>
#include <assert.h>
#include <STDDEF.H>

/*
 * For NMEA-conforming receivers this is supposed to be 82, but
 * some receivers (TN-200, GSW 2.3.2) emit oversized sentences.
 * The current hog champion is the Trimble BX-960 receiver, which
 * emits a 91-character GGA message.
 */
#define NMEA_MAX	91		/* max length of NMEA sentence */

/*AIS数据结构体——26类信息*/
struct ais_t
{
	unsigned int	type;		/* message type */
	unsigned int    repeat;		/* Repeat indicator */
	unsigned int	mmsi;		/* MMSI */
	union {
		/* Types 1-3 Common navigation info */
		struct {
			unsigned int status;		/* navigation status */
			signed turn;			/* rate of turn */
#define AIS_TURN_HARD_LEFT	-127
#define AIS_TURN_HARD_RIGHT	127
#define AIS_TURN_NOT_AVAILABLE	128
			unsigned int speed;			/* speed over ground in deciknots */
#define AIS_SPEED_NOT_AVAILABLE	1023
#define AIS_SPEED_FAST_MOVER	1022		/* >= 102.2 knots */
			bool accuracy;			/* position accuracy */
#define AIS_LATLON_SCALE	600000.0
			int lon;				/* longitude */
#define AIS_LON_NOT_AVAILABLE	0x6791AC0
			int lat;				/* latitude */
#define AIS_LAT_NOT_AVAILABLE	0x3412140
			unsigned int course;		/* course over ground */
#define AIS_COURSE_NOT_AVAILABLE	3600
			unsigned int heading;		/* true heading */
#define AIS_HEADING_NOT_AVAILABLE	511
			unsigned int second;		/* seconds of UTC timestamp */
#define AIS_SEC_NOT_AVAILABLE	60
#define AIS_SEC_MANUAL		61
#define AIS_SEC_ESTIMATED	62
#define AIS_SEC_INOPERATIVE	63
			unsigned int maneuver;	/* maneuver indicator */
			//unsigned int spare;	spare bits */
			bool raim;			/* RAIM flag */
			unsigned int radio;		/* radio status bits */
		} type1;
		/* Type 4 - Base Station Report & Type 11 - UTC and Date Response */
		struct {
			unsigned int year;			/* UTC year */
#define AIS_YEAR_NOT_AVAILABLE	0
			unsigned int month;			/* UTC month */
#define AIS_MONTH_NOT_AVAILABLE	0
			unsigned int day;			/* UTC day */
#define AIS_DAY_NOT_AVAILABLE	0
			unsigned int hour;			/* UTC hour */
#define AIS_HOUR_NOT_AVAILABLE	24
			unsigned int minute;		/* UTC minute */
#define AIS_MINUTE_NOT_AVAILABLE	60
			unsigned int second;		/* UTC second */
#define AIS_SECOND_NOT_AVAILABLE	60
			bool accuracy;		/* fix quality */
			int lon;			/* longitude */
			int lat;			/* latitude */
			unsigned int epfd;		/* type of position fix device */
			//unsigned int spare;	spare bits */
			bool raim;			/* RAIM flag */
			unsigned int radio;		/* radio status bits */
		} type4;
		/* Type 5 - Ship static and voyage related data */
		struct {
			unsigned int ais_version;	/* AIS version level */
			unsigned int imo;		/* IMO identification */
			char callsign[8];		/* callsign */
#define AIS_SHIPNAME_MAXLEN	20
			char shipname[AIS_SHIPNAME_MAXLEN + 1];	/* vessel name */
			unsigned int shiptype;	/* ship type code */
			unsigned int to_bow;	/* dimension to bow */
			unsigned int to_stern;	/* dimension to stern */
			unsigned int to_port;	/* dimension to port */
			unsigned int to_starboard;	/* dimension to starboard */
			unsigned int epfd;		/* type of position fix deviuce */
			unsigned int month;		/* UTC month */
			unsigned int day;		/* UTC day */
			unsigned int hour;		/* UTC hour */
			unsigned int minute;	/* UTC minute */
			unsigned int draught;	/* draft in meters */
			char destination[21];	/* ship destination */
			unsigned int dte;		/* data terminal enable */
			//unsigned int spare;	spare bits */
		} type5;
		/* Type 6 - Addressed Binary Message */
		struct {
			unsigned int seqno;		/* sequence number */
			unsigned int dest_mmsi;	/* destination MMSI */
			bool retransmit;		/* retransmit flag */
			//unsigned int spare;	spare bit(s) */
			unsigned int app_id;        /* Application ID */
#define AIS_TYPE6_BINARY_MAX	920	/* 920 bits */
			size_t bitcount;		/* bit count of the data */
			char bitdata[(AIS_TYPE6_BINARY_MAX + 7) / 8];
		} type6;
		/* Type 7 - Binary Acknowledge */
		struct {
			unsigned int mmsi1;
			unsigned int mmsi2;
			unsigned int mmsi3;
			unsigned int mmsi4;
			/* spares ignored, they're only padding here */
		} type7;
		/* Type 8 - Broadcast Binary Message */
		struct {
			//unsigned int spare;	spare bit(s) */
			unsigned int app_id;       	/* Application ID */
#define AIS_TYPE8_BINARY_MAX	952	/* 952 bits */
			size_t bitcount;		/* bit count of the data */
			char bitdata[(AIS_TYPE8_BINARY_MAX + 7) / 8];
		} type8;
		/* Type 9 - Standard SAR Aircraft Position Report */
		struct {
			unsigned int alt;		/* altitude in meters */
#define AIS_ALT_NOT_AVAILABLE	4095
#define AIS_ALT_HIGH    	4094	/* 4094 meters or higher */
			unsigned int speed;		/* speed over ground in deciknots */
#define AIS_SAR_SPEED_NOT_AVAILABLE	1023
#define AIS_SAR_FAST_MOVER  	1022
			bool accuracy;		/* position accuracy */
			int lon;			/* longitude */
			int lat;			/* latitude */
			unsigned int course;	/* course over ground */
			unsigned int second;	/* seconds of UTC timestamp */
			unsigned int regional;	/* regional reserved */
			unsigned int dte;		/* data terminal enable */
			//unsigned int spare;	spare bits */
			bool assigned;		/* assigned-mode flag */
			bool raim;			/* RAIM flag */
			unsigned int radio;		/* radio status bits */
		} type9;
		/* Type 10 - UTC/Date Inquiry */
		struct {
			//unsigned int spare;
			unsigned int dest_mmsi;	/* destination MMSI */
			//unsigned int spare2;
		} type10;
		/* Type 12 - Safety-Related Message */
		struct {
			unsigned int seqno;		/* sequence number */
			unsigned int dest_mmsi;	/* destination MMSI */
			bool retransmit;		/* retransmit flag */
			//unsigned int spare;	spare bit(s) */
#define AIS_TYPE12_TEXT_MAX	157	/* 936 bits of six-bit, plus NUL */
			char text[AIS_TYPE12_TEXT_MAX];
		} type12;
		/* Type 14 - Safety-Related Broadcast Message */
		struct {
			//unsigned int spare;	spare bit(s) */
#define AIS_TYPE14_TEXT_MAX	161	/* 952 bits of six-bit, plus NUL */
			char text[AIS_TYPE14_TEXT_MAX];
		} type14;
		/* Type 15 - Interrogation */
		struct {
			//unsigned int spare;	spare bit(s) */
			unsigned int mmsi1;
			unsigned int type1_1;
			unsigned int offset1_1;
			//unsigned int spare2;	spare bit(s) */
			unsigned int type1_2;
			unsigned int offset1_2;
			//unsigned int spare3;	spare bit(s) */
			unsigned int mmsi2;
			unsigned int type2_1;
			unsigned int offset2_1;
			//unsigned int spare4;	spare bit(s) */
		} type15;
		/* Type 16 - Assigned Mode Command */
		struct {
			//unsigned int spare;	spare bit(s) */
			unsigned int mmsi1;
			unsigned int offset1;
			unsigned int increment1;
			unsigned int mmsi2;
			unsigned int offset2;
			unsigned int increment2;
		} type16;
		/* Type 17 - GNSS Broadcast Binary Message */
		struct {
			//unsigned int spare;	spare bit(s) */
#define AIS_GNSS_LATLON_SCALE	600.0
			int lon;			/* longitude */
			int lat;			/* latitude */
			//unsigned int spare2;	spare bit(s) */
#define AIS_TYPE17_BINARY_MAX	736	/* 920 bits */
			size_t bitcount;		/* bit count of the data */
			char bitdata[(AIS_TYPE17_BINARY_MAX + 7) / 8];
		} type17;
		/* Type 18 - Standard Class B CS Position Report */
		struct {
			unsigned int reserved;	/* altitude in meters */
			unsigned int speed;		/* speed over ground in deciknots */
			bool accuracy;		/* position accuracy */
			int lon;			/* longitude */
#define AIS_GNS_LON_NOT_AVAILABLE	0x1a838
			int lat;			/* latitude */
#define AIS_GNS_LAT_NOT_AVAILABLE	0xd548
			unsigned int course;	/* course over ground */
			unsigned int heading;	/* true heading */
			unsigned int second;	/* seconds of UTC timestamp */
			unsigned int regional;	/* regional reserved */
			bool cs;     		/* carrier sense unit flag */
			bool display;		/* unit has attached display? */
			bool dsc;   		/* unit attached to radio with DSC? */
			bool band;   		/* unit can switch frequency bands? */
			bool msg22;	        	/* can accept Message 22 management? */
			bool assigned;		/* assigned-mode flag */
			bool raim;			/* RAIM flag */
			unsigned int radio;		/* radio status bits */
		} type18;
		/* Type 19 - Extended Class B CS Position Report */
		struct {
			unsigned int reserved;	/* altitude in meters */
			unsigned int speed;		/* speed over ground in deciknots */
			bool accuracy;		/* position accuracy */
			int lon;			/* longitude */
			int lat;			/* latitude */
			unsigned int course;	/* course over ground */
			unsigned int heading;	/* true heading */
			unsigned int second;	/* seconds of UTC timestamp */
			unsigned int regional;	/* regional reserved */
			char shipname[AIS_SHIPNAME_MAXLEN + 1];		/* ship name */
			unsigned int shiptype;	/* ship type code */
			unsigned int to_bow;	/* dimension to bow */
			unsigned int to_stern;	/* dimension to stern */
			unsigned int to_port;	/* dimension to port */
			unsigned int to_starboard;	/* dimension to starboard */
			unsigned int epfd;		/* type of position fix deviuce */
			bool raim;			/* RAIM flag */
			unsigned int dte;    	/* date terminal enable */
			bool assigned;		/* assigned-mode flag */
			//unsigned int spare;	spare bits */
		} type19;
		/* Type 20 - Data Link Management Message */
		struct {
			//unsigned int spare;	spare bit(s) */
			unsigned int offset1;	/* TDMA slot offset */
			unsigned int number1;	/* number of xlots to allocate */
			unsigned int timeout1;	/* allocation timeout */
			unsigned int increment1;	/* repeat increment */
			unsigned int offset2;	/* TDMA slot offset */
			unsigned int number2;	/* number of xlots to allocate */
			unsigned int timeout2;	/* allocation timeout */
			unsigned int increment2;	/* repeat increment */
			unsigned int offset3;	/* TDMA slot offset */
			unsigned int number3;	/* number of xlots to allocate */
			unsigned int timeout3;	/* allocation timeout */
			unsigned int increment3;	/* repeat increment */
			unsigned int offset4;	/* TDMA slot offset */
			unsigned int number4;	/* number of xlots to allocate */
			unsigned int timeout4;	/* allocation timeout */
			unsigned int increment4;	/* repeat increment */
		} type20;
		/* Type 21 - Aids to Navigation Report */
		struct {
			unsigned int aid_type;	/* aid type */
			char name[35];		/* name of aid to navigation */
			bool accuracy;		/* position accuracy */
			int lon;			/* longitude */
			int lat;			/* latitude */
			unsigned int to_bow;	/* dimension to bow */
			unsigned int to_stern;	/* dimension to stern */
			unsigned int to_port;	/* dimension to port */
			unsigned int to_starboard;	/* dimension to starboard */
			unsigned int epfd;		/* type of EPFD */
			unsigned int second;	/* second of UTC timestamp */
			bool off_position;		/* off-position indicator */
			unsigned int regional;	/* regional reserved field */
			bool raim;			/* RAIM flag */
			bool virtual_aid;		/* is virtual station? */
			bool assigned;		/* assigned-mode flag */
			//unsigned int spare;	unused */
		} type21;
		/* Type 22 - Channel Management */
		struct {
			//unsigned int spare;	spare bit(s) */
			unsigned int channel_a;	/* Channel A number */
			unsigned int channel_b;	/* Channel B number */
			unsigned int txrx;		/* transmit/receive mode */
			bool power;			/* high-power flag */
#define AIS_CHANNEL_LATLON_SCALE	600.0
			union {
				struct {
					int ne_lon;		/* NE corner longitude */
					int ne_lat;		/* NE corner latitude */
					int sw_lon;		/* SW corner longitude */
					int sw_lat;		/* SW corner latitude */
				} area;
				struct {
					unsigned int dest1;	/* addressed station MMSI 1 */
					unsigned int dest2;	/* addressed station MMSI 2 */
				} mmsi;
			};
			bool addressed;		/* addressed vs. broadast flag */
			bool band_a;		/* fix 1.5kHz band for channel A */
			bool band_b;		/* fix 1.5kHz band for channel B */
			unsigned int zonesize;	/* size of transitional zone */
		} type22;
		/* Type 23 - Group Assignment Command */
		struct {
			int ne_lon;			/* NE corner longitude */
			int ne_lat;			/* NE corner latitude */
			int sw_lon;			/* SW corner longitude */
			int sw_lat;			/* SW corner latitude */
			//unsigned int spare;	spare bit(s) */
			unsigned int stationtype;	/* station type code */
			unsigned int shiptype;	/* ship type code */
			//unsigned int spare2;	spare bit(s) */
			unsigned int txrx;		/* transmit-enable code */
			unsigned int interval;	/* report interval */
			unsigned int quiet;		/* quiet time */
			//unsigned int spare3;	spare bit(s) */
		} type23;
		/* Type 24 - Class B CS Static Data Report */
		struct {
			char shipname[AIS_SHIPNAME_MAXLEN + 1];	/* vessel name */
			unsigned int shiptype;	/* ship type code */
			char vendorid[8];		/* vendor ID */
			char callsign[8];		/* callsign */
			union {
				unsigned int mothership_mmsi;	/* MMSI of main vessel */
				struct {
					unsigned int to_bow;	/* dimension to bow */
					unsigned int to_stern;	/* dimension to stern */
					unsigned int to_port;	/* dimension to port */
					unsigned int to_starboard;	/* dimension to starboard */
				} dim;
			};
		} type24;
		/* Type 25 - Addressed Binary Message */
		struct {
			bool addressed;		/* addressed-vs.broadcast flag */
			bool structured;		/* structured-binary flag */
			unsigned int dest_mmsi;	/* destination MMSI */
			unsigned int app_id;        /* Application ID */
#define AIS_TYPE25_BINARY_MAX	128	/* Up to 128 bits */
			size_t bitcount;		/* bit count of the data */
			char bitdata[(AIS_TYPE25_BINARY_MAX + 7) / 8];
		} type25;
		/* Type 26 - Addressed Binary Message */
		struct {
			bool addressed;		/* addressed-vs.broadcast flag */
			bool structured;		/* structured-binary flag */
			unsigned int dest_mmsi;	/* destination MMSI */
			unsigned int app_id;        /* Application ID */
#define AIS_TYPE26_BINARY_MAX	1004	/* Up to 128 bits */
			size_t bitcount;		/* bit count of the data */
			char bitdata[(AIS_TYPE26_BINARY_MAX + 7) / 8];
			unsigned int radio;		/* radio status bits */
		} type26;
	};
};

/*AIVDM结构体——sentence的第6个字段内容（payload）*/
struct aivdm_context_t {
	/* hold context for decoding AIDVM packet sequences */
	int part, await;		/* for tracking AIDVM parts in a multipart sequence */
	unsigned char *field[NMEA_MAX];
	unsigned char fieldcopy[NMEA_MAX + 1];
	unsigned char bits[2048];
	char shipname[AIS_SHIPNAME_MAXLEN + 1];
	size_t bitlen;
};

/*船舶状态*/
struct attitude_t {
	double heading;
	double pitch;
	double roll;
	double yaw;
	double dip;
	double mag_len; /* unitvector sqrt(x^2 + y^2 +z^2) */
	double mag_x;
	double mag_y;
	double mag_z;
	double acc_len; /* unitvector sqrt(x^2 + y^2 +z^2) */
	double acc_x;
	double acc_y;
	double acc_z;
	double gyro_x;
	double gyro_y;
	double temp;
	double depth;
	/* compass status -- TrueNorth (and any similar) devices only */
	char mag_st;
	char pitch_st;
	char roll_st;
	char yaw_st;
};

/*
 * Is an MMSI number that of an auxiliary associated with a mother ship?
 * We need to be able to test this for decoding AIS Type 24 messages.
 * According to <http://www.navcen.uscg.gov/marcomms/gmdss/mmsi.htm#format>,
 * auxiliary-craft MMSIs have the form 98MIDXXXX, where MID is a country
 * code and XXXX the vessel ID.
 */
#define AIS_AUXILIARY_MMSI(n)	((n) / 10000000 == 98)



//函数接口说明
size_t strlcpy(char *dst, const char *src, size_t siz);
unsigned __int64 ubits(char buf[], unsigned int start, unsigned int width);
signed __int64 sbits(char buf[], unsigned int start, unsigned int width);
static void from_sixbit(char *bitvec, unsigned int start, int count, char *to);
bool aivdmo_decode(const char *buf, size_t buflen, struct aivdm_context_t *ais_context, struct ais_t *ais);
