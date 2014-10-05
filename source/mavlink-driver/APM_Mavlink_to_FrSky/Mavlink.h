/*
   @author    Nils Hцgberg
   @contact    nils.hogberg@gmail.com
    @coauthor(s):
     Victor Brutskiy, 4refr0nt@gmail.com, er9x adaptation

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef mavlink_h
#define mavlink_h

#include "ifrskydataprovider.h"
#include "SoftwareSerial.h"
#include <FastSerial.h>
//#include <mavlink.h>
#include <GCS_MAVLink.h>
//#include <../Libraries/GCS_MAVLink/GCS_MAVLink.h>
//#include "../GCS_MAVLink/include/mavlink/v1.0/mavlink_types.h"
//#include "../GCS_MAVLink/include/mavlink/v1.0/ardupilotmega/mavlink.h"
#include "defines.h"

class Mavlink :   public IFrSkyDataProvider
{
public:
   Mavlink(BetterStream* port);
   ~Mavlink(void);
   int             parseMessage(char c);
   void            makeRateRequest();
   bool            enable_mav_request;
   unsigned long   lastMAVBeat;
   const int       getGpsStatus();
   unsigned int    msg_timer;
   void            printMessage(SoftwareSerial* serialPort, IFrSkyDataProvider* dataProvider, int msg);

   // IFrSkyDataProvider functions
   const float     getGpsAltitude();
   const int       getTemp1();
   const int       getEngineSpeed();
   const int       getFuelLevel();
   const int       getTemp2();
   const float     getAltitude();
   const float     getGpsGroundSpeed();
   const float     getLongitud();
   const float     getLatitude();
   const float     getCourse();
   const int       getYear();
   const int       getDate();
   const int       getTime();
//   const float     getAccX();
//   const float     getAccY();
//   const float     getAccZ(); 
   const float     getBatteryCurrent();
   const float     getMainBatteryVoltage();
   void            reset();
   const int       getBaseMode();
   const int       getWP_dist();
   const int       getWP_num();
   const int       getWP_bearing();
   const int       getHealth();
   const int       getHome_dir();
   const int       getHome_dist();
   const int       getCpu_load();
   const int       getVcc();
   const int       getGpsHdop();
   const int       getStatus_msg();
   const int       getNCell();
   const int       getCell();
   const int       parse_msg();
   const bool      isArmed();
   void            setHomeVars();
   boolean         getBit(byte Reg, byte whichBit);
   float           home_gps_alt;
   float           alt;
   float           gps_alt;
   float           home_lat;
   float           home_lon;
   float           lat;
   float           lon;
   bool            ok;
   bool            motor_armed;
   unsigned int    gpsHdop;
   int             gpsStatus;
   unsigned int    status_msg;
   int             apmMode;
private:
   SoftwareSerial  *debugPort;
   float           gpsDdToDmsFormat(float ddm);
   bool            mavbeat;
   unsigned int    apm_mav_type;
   unsigned int    apm_mav_system; 
   unsigned int    apm_mav_component;
   unsigned int    crlf_count;
   int             packet_drops;
   int             parse_error;
   double          scaleLongDown;
   double          scaleLongUp;
   float           rads;
   const bool      compare_msg(char *string);

   // Telemetry values
   float           batteryVoltage;
   int             current;
   int             batteryRemaining;
   float           latitude;
   float           longitude;
   float           gpsAltitude;
   int             numberOfSatelites;
   float           gpsGroundSpeed;
   float           gpsCourse;
   float           altitude;
   int             apmBaseMode;
   int             course;
   int             home_course;
   unsigned int    throttle;
//   float           accX;
//   float           accY;
//   float           accZ;
   int             sensors_health;
   int             cpu_load;
   int             cpu_vcc;
   unsigned int    wp_dist;
   int             wp_num;
   int             wp_bearing;
   int             home_direction;
   unsigned int    home_distance;
   int             last_message_severity;
   char            last_message_text[50];
   unsigned int    ncell;
   unsigned int    cell;
};
#define MAV_SYS_STATUS_SENSOR_3D_GYRO 1 /* 0x01 3D gyro | */
#define MAV_SYS_STATUS_SENSOR_3D_ACCEL 2 /* 0x02 3D accelerometer | */
#define MAV_SYS_STATUS_SENSOR_3D_MAG 4  /* 0x04 3D magnetometer | */
#define MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE 8 /* 0x08 absolute pressure | */
#define MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE 16 /* 0x10 differential pressure | */
#define MAV_SYS_STATUS_SENSOR_GPS 32 /* 0x20 GPS | */
#define MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW 64 /* 0x40 optical flow | */
#define MAV_SYS_STATUS_SENSOR_VISION_POSITION 128 /* 0x80 computer vision position | */
#define MAV_SYS_STATUS_SENSOR_LASER_POSITION 256 /* 0x100 laser based position | */
#define MAV_SYS_STATUS_SENSOR_EXTERNAL_GROUND_TRUTH 512 /* 0x200 external ground truth (Vicon or Leica) | */
#define MAV_SYS_STATUS_SENSOR_ANGULAR_RATE_CONTROL 1024 /* 0x400 3D angular rate control | */
#define MAV_SYS_STATUS_SENSOR_ATTITUDE_STABILIZATION 2048 /* 0x800 attitude stabilization | */
#define MAV_SYS_STATUS_SENSOR_YAW_POSITION 4096 /* 0x1000 yaw position | */
#define MAV_SYS_STATUS_SENSOR_Z_ALTITUDE_CONTROL 8192 /* 0x2000 z/altitude control | */
#define MAV_SYS_STATUS_SENSOR_XY_POSITION_CONTROL 16384 /* 0x4000 x/y position control | */
#define MAV_SYS_STATUS_SENSOR_MOTOR_OUTPUTS 32768 /* 0x8000 motor outputs / control | */
#define MAV_SYS_STATUS_SENSOR_RC_RECEIVER 65536 /* 0x10000 rc receiver | */
#define MAV_SYS_STATUS_SENSOR_3D_GYRO2 131072 /* 0x20000 2nd 3D gyro | */
#define MAV_SYS_STATUS_SENSOR_3D_ACCEL2 262144 /* 0x40000 2nd 3D accelerometer | */
#define MAV_SYS_STATUS_SENSOR_3D_MAG2 524288 /* 0x80000 2nd 3D magnetometer | */
#define MAV_SYS_STATUS_GEOFENCE 1048576 /* 0x100000 geofence | */
#define MAV_SYS_STATUS_AHRS 2097152 /* 0x200000 AHRS subsystem health | */
#endif
