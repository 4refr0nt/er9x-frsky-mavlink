/*
   @author    Nils Hцgberg
   @contact    nils.hogberg@gmail.com
    @coauthor(s):
     Victor Brutskiy, 4refr0nt@gmail.com, er9x adaptation

   Original code from https://code.google.com/p/arducam-osd/wiki/arducam_osd

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

#include "Mavlink.h"

//#include "include/mavlink/v1.0/mavlink_types.h"
//#include "include/mavlink/v1.0/ardupilotmega/mavlink.h"

//#include "../Libraries/GCS_MAVLink/GCS_MAVLink.h"
//#include "../GCS_MAVLink/include/mavlink/v1.0/mavlink_types.h"
//#include "../GCS_MAVLink/include/mavlink/v1.0/ardupilotmega/mavlink.h"

// check actual data at https://github.com/diydrones/ardupilot/search?q=SEVERITY_HIGH
char MSG01[] PROGMEM = "ARMING MOTORS"; 
char MSG02[] PROGMEM = "PreArm: RC not calibrated"; 
char MSG03[] PROGMEM = "PreArm: Baro not healthy";
char MSG04[] PROGMEM = "PreArm: Compass not healthy";
char MSG05[] PROGMEM = "PreArm: Bad GPS Pos";
char MSG06[] PROGMEM = "compass disabled";
char MSG07[] PROGMEM = "check compass";
char MSG08[] PROGMEM = "Motor Test: RC not calibrated";
char MSG09[] PROGMEM = "Motor Test: vehicle not landed";
char MSG10[] PROGMEM = "Crash: Disarming";
char MSG11[] PROGMEM = "Parachute: Released";
char MSG12[] PROGMEM = "error setting rally point";
char MSG13[] PROGMEM = "AutoTune: Started";
char MSG14[] PROGMEM = "AutoTune: Stopped";
char MSG15[] PROGMEM = "Trim saved";
char MSG16[] PROGMEM = "EKF variance";
char MSG17[] PROGMEM = "verify_nav: invalid or no current nav";
char MSG18[] PROGMEM = "verify_conditon: invalid or no current condition";
char MSG19[] PROGMEM = "Disable fence failed (autodisable)";
char MSG20[] PROGMEM = "ESC Cal: restart board";
char MSG21[] PROGMEM = "ESC Cal: passing pilot thr to ESCs";
char MSG22[] PROGMEM = "Low Battery";
char MSG23[] PROGMEM = "Lost GPS";
char MSG24[] PROGMEM = "Fence disabled (autodisable)";
char *MsgPointers[] PROGMEM = {MSG01, MSG02, MSG03, MSG04, MSG05, MSG06, MSG07, MSG08, MSG09, MSG10, MSG11, MSG12, MSG13, MSG14, MSG15, MSG16, MSG17, MSG18, MSG19, MSG20, MSG21, MSG22, MSG23, MSG24};
#define MAX_MSG 24

Mavlink::Mavlink(BetterStream* port)
{
   mavlink_comm_0_port = port;
   mavlink_system.sysid = 12;
   mavlink_system.compid = 1;
   mavlink_system.type = 0;
   mavlink_system.state = 0;
   
   lastMAVBeat = 0;
   enable_mav_request = 1;
   msg_timer = 0;

   batteryVoltage = 0;
   current = 0;
   batteryRemaining = 0;
   gpsStatus = 0;
   latitude = 0;
   longitude = 0;
   gpsAltitude = 0;
   gpsHdop = 999;
   numberOfSatelites = 0;
   gpsGroundSpeed = 0;
   gpsCourse = 0;
   altitude = 0.0f;
   apmMode = 99;
   course = 0;
   home_course = 0;
   throttle = 0;
//   accX = 0;
//   accY = 0;
//   accZ = 0;
   apmBaseMode = 0;
   wp_dist = 0;
   wp_num = 0;
   wp_bearing = 270;
   sensors_health = 0;
   last_message_severity = 0;
   home_gps_alt = 0;
   alt = 0;
   gps_alt = 0;
   home_distance = 0;
   home_direction = 90;
   lat = 0;
   lon = 0;
   cpu_load = 0;
   cpu_vcc = 0;
   status_msg = 0;
   motor_armed = 0;
   
}

Mavlink::~Mavlink(void)
{
}

const float Mavlink::getMainBatteryVoltage()
{
   return batteryVoltage;
}

const float Mavlink::getBatteryCurrent()
{
   return current;
}

const int Mavlink::getFuelLevel()
{
   return batteryRemaining;
}

const int Mavlink::getGpsStatus()
{
   return gpsStatus;
}

const float Mavlink::getLatitude()
{
//   return gpsDdToDmsFormat(latitude / 10000000.0f);
   return latitude / 10000000.0f;
}

const float Mavlink::getLongitud()
{
//   return gpsDdToDmsFormat(longitude / 10000000.0f);
   return longitude / 10000000.0f;
}

const float Mavlink::getGpsAltitude()
{
   setHomeVars();
   return gps_alt/1000; //gpsAltitude / 1000;
}

const int Mavlink::getGpsHdop()
{
   if ( gpsHdop > 999 ) gpsHdop = 999;
   return gpsHdop;
}

const int Mavlink::getTemp2()
{   
   return numberOfSatelites * 10 + gpsStatus;
}

const float Mavlink::getGpsGroundSpeed()
{
   return gpsGroundSpeed; //* 0.0194384f;
}

const float Mavlink::getAltitude()
{
   return altitude;
}

const int Mavlink::getTemp1()
{
   return apmMode;
}

const bool Mavlink::isArmed()
{
    return motor_armed;
}
const float Mavlink::getCourse()
{
   int new_course; 
   if ( isArmed() ){
      new_course = course - home_course;
	} else {
	  new_course = course;
	  home_course = course;
	}
   if (new_course < 0) { 
	    new_course += 360;
	}
    return new_course; // without normalization
}

const int Mavlink::getEngineSpeed()
{
   return throttle;
}

/*
const float Mavlink::getAccX()
{
   return accX;
}
   
const float Mavlink::getAccY()
{
   return accY;
}

const float Mavlink::getAccZ()
{
   return accZ;
}

const int Mavlink::getYear()
{
   return 0;
}

const int Mavlink::getTime()
{
   return 0;
}

const int Mavlink::getDate()
{
   return 0;
}
*/
const int Mavlink::getBaseMode()
{
   return apmBaseMode;
}
const int Mavlink::getWP_dist()
{
   return wp_dist;
}
const int Mavlink::getWP_num()
{
   return wp_num;
}
const int Mavlink::getWP_bearing()
{
   int new_bearing = wp_bearing + 270;
   if (new_bearing >= 360) {
	  new_bearing -= 360;
   }
   return new_bearing; // normalized value
}
const int Mavlink::getHealth()
{
   return sensors_health;
}
const int Mavlink::getHome_dir()
{
   setHomeVars();
   return home_direction; // normalized value
}
const int Mavlink::getHome_dist()
{
   setHomeVars();
   return home_distance;
}
const int Mavlink::getCpu_load()
{
   return cpu_load;
}
const int Mavlink::getVcc()
{
   return cpu_vcc;
}
const int Mavlink::getStatus_msg()
{
	if ( msg_timer <= 0 ){ 
        status_msg = 0;
	}
    return status_msg;
}
const int Mavlink::parse_msg()
{
	int i = 0;
	bool success = false;
    while ( (i <= MAX_MSG) && !success )	{
      if ( compare_msg((char*)pgm_read_word(&(MsgPointers[i]))) ) {
	     success = true;
	  }
	  i++;
	}
	if (success) {
       return i;
	} else {
       return 99;
	}
}
const bool Mavlink::compare_msg(char *string)
{
    bool eq = true;
	int i = 0;
    while (pgm_read_byte(string)!='\0' && eq)
	{
	   if ( pgm_read_byte(string) == last_message_text[i] )
	   {
	      // do nothing
	   } else {
	     eq = false;
	   }
	   string++;
	   i++;
	}
	return eq;
}
const int Mavlink::getNCell()
{
   return ncell;
}
const int Mavlink::getCell()
{
   return cell;
}

// We receive the GPS coordinates in ddd.dddd format
// FrSky wants the dd mm.mmm format so convert.
float Mavlink::gpsDdToDmsFormat(float ddm)
{
   int deg = (int)ddm;
   float min_dec = (ddm - deg) * 60.0f;
   float sec = (min_dec - (int)min_dec) * 60.0f;

   return (float)deg * 100.0f + (int)min_dec + sec / 100.0f;
}

void Mavlink::makeRateRequest()
{
    const int  maxStreams = 7;
    const unsigned short MAVStreams[maxStreams] = {
        MAV_DATA_STREAM_RAW_SENSORS,
        MAV_DATA_STREAM_EXTENDED_STATUS,
        MAV_DATA_STREAM_RC_CHANNELS,
        MAV_DATA_STREAM_POSITION,
        MAV_DATA_STREAM_EXTRA1, 
        MAV_DATA_STREAM_EXTRA2,
        MAV_DATA_STREAM_EXTRA3
    };
    const unsigned int MAVRates[maxStreams] = {0x02, 0x02, 0x02, 0x02, 0x05, 0x05, 0x02};
    for (int i=0; i < maxStreams; i++) {
        mavlink_msg_request_data_stream_send(MAVLINK_COMM_0,
            apm_mav_system, apm_mav_component,
            MAVStreams[i], MAVRates[i], 1);
    }
}
/*
    if (stream_trigger(STREAM_RAW_SENSORS)) {
        send_message(MSG_RAW_IMU1);
        send_message(MSG_RAW_IMU2);
        send_message(MSG_RAW_IMU3);
    }
    if (stream_trigger(STREAM_EXTENDED_STATUS)) {
        send_message(MSG_EXTENDED_STATUS1);
        send_message(MSG_EXTENDED_STATUS2);
        send_message(MSG_CURRENT_WAYPOINT);
        send_message(MSG_GPS_RAW);
        send_message(MSG_NAV_CONTROLLER_OUTPUT);
        send_message(MSG_LIMITS_STATUS);
    }
    if (stream_trigger(STREAM_POSITION)) {
        send_message(MSG_LOCATION);
    }
    if (stream_trigger(STREAM_RAW_CONTROLLER)) {
        send_message(MSG_SERVO_OUT);
    }
    if (stream_trigger(STREAM_RC_CHANNELS)) {
        send_message(MSG_RADIO_OUT);
        send_message(MSG_RADIO_IN);
    }
    if (stream_trigger(STREAM_EXTRA1)) {
        send_message(MSG_ATTITUDE);
        send_message(MSG_SIMSTATE);
    }
    if (stream_trigger(STREAM_EXTRA2)) {
        send_message(MSG_VFR_HUD);
    }
    if (stream_trigger(STREAM_EXTRA3)) {
        send_message(MSG_AHRS);
        send_message(MSG_HWSTATUS);
        send_message(MSG_SYSTEM_TIME);
        send_message(MSG_RANGEFINDER);
    }

*/
int Mavlink::parseMessage(char c)
{
    mavlink_message_t msg; 
    mavlink_status_t status;
    unsigned long sensors_enabled;
    unsigned long health;

    //trying to grab msg  
   if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) 
   {
         switch(msg.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
            {
                apm_mav_system    = msg.sysid;
                apm_mav_component = msg.compid;
                apm_mav_type      = mavlink_msg_heartbeat_get_type(&msg);            
                apmMode             = (unsigned int)mavlink_msg_heartbeat_get_custom_mode(&msg);
                apmBaseMode       = mavlink_msg_heartbeat_get_base_mode(&msg);
                if (getBit(apmBaseMode, MOTORS_ARMED)) {
                   motor_armed = 1;
                } else {
                   motor_armed = 0;
                }
                return MAVLINK_MSG_ID_HEARTBEAT;
            }
            break;
         case MAVLINK_MSG_ID_SYS_STATUS:
            {
                batteryVoltage = (mavlink_msg_sys_status_get_voltage_battery(&msg) / 1000.0f); // Volts, Battery voltage, in millivolts (1 = 1 millivolt)
                ncell = batteryVoltage / 4.3f;
                ncell++;
				if (ncell > 5) ncell = 5;
                cell = 500 * (batteryVoltage / float(ncell));
				if (cell < 1500) cell = 0;
                current          = mavlink_msg_sys_status_get_current_battery(&msg) / 10; //0.1A Battery current, in 10*milliamperes (1 = 10 milliampere)         
                batteryRemaining = mavlink_msg_sys_status_get_battery_remaining(&msg); //Remaining battery energy: (0%: 0, 100%: 100)
                sensors_enabled  = mavlink_msg_sys_status_get_onboard_control_sensors_enabled(&msg);
                health           = mavlink_msg_sys_status_get_onboard_control_sensors_health(&msg); // Bitmask showing which onboard controllers and sensors are operational or have an error:  Value of 0: not enabled. Value of 1: enabled. Indices defined by ENUM MAV_SYS_STATUS_SENSOR
                cpu_load         = mavlink_msg_sys_status_get_load(&msg) / 10; // Maximum usage in percent of the mainloop time, (0%: 0, 100%: 1000) should be always below 1000
                sensors_health   = 0; // Default - All sensors status: ok
                if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_3D_GYRO )
                if (!( health & MAV_SYS_STATUS_SENSOR_3D_GYRO ))                    sensors_health |= MAV_SYS_STATUS_SENSOR_3D_GYRO;
                if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_3D_ACCEL )
                if (!( health & MAV_SYS_STATUS_SENSOR_3D_ACCEL ))                   sensors_health |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
                if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_3D_MAG )
                if (!( health & MAV_SYS_STATUS_SENSOR_3D_MAG ))                     sensors_health |= MAV_SYS_STATUS_SENSOR_3D_MAG;
                if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE )
                if (!( health & MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE ))          sensors_health |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
                if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE )
                if (!( health & MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE ))      sensors_health |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
                if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_GPS )
                if (!( health & MAV_SYS_STATUS_SENSOR_GPS ))                        sensors_health |= MAV_SYS_STATUS_SENSOR_GPS;
                if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW )
                if (!( health & MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW ))               sensors_health |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
                if ( sensors_enabled & MAV_SYS_STATUS_GEOFENCE )
                if (!( health & MAV_SYS_STATUS_GEOFENCE ))                          sensors_health |= MAV_SYS_STATUS_GEOFENCE;
                if ( sensors_enabled & MAV_SYS_STATUS_AHRS )
                if (!( health & MAV_SYS_STATUS_AHRS ))                              sensors_health |= MAV_SYS_STATUS_AHRS;
                return MAVLINK_MSG_ID_SYS_STATUS;
            }
            break;
            case MAVLINK_MSG_ID_GPS_RAW_INT:
            {
                latitude          = mavlink_msg_gps_raw_int_get_lat(&msg);
                longitude         = mavlink_msg_gps_raw_int_get_lon(&msg);
                gpsStatus         = mavlink_msg_gps_raw_int_get_fix_type(&msg);
                numberOfSatelites = mavlink_msg_gps_raw_int_get_satellites_visible(&msg);
                gpsHdop           = mavlink_msg_gps_raw_int_get_eph(&msg);
                gpsAltitude       = mavlink_msg_gps_raw_int_get_alt(&msg); // meters * 1000
                //gpsCourse         = mavlink_msg_gps_raw_int_get_cog(&msg);
                return MAVLINK_MSG_ID_GPS_RAW_INT;
            }
            break;
            case MAVLINK_MSG_ID_VFR_HUD:
            {
                lastMAVBeat = millis(); // we waiting only HUD packet
                //airspeed = mavlink_msg_vfr_hud_get_airspeed(&msg);
                gpsGroundSpeed = mavlink_msg_vfr_hud_get_groundspeed(&msg); // Current ground speed in m/s
                course = mavlink_msg_vfr_hud_get_heading(&msg); // 0..360 deg, 0=north
                throttle = mavlink_msg_vfr_hud_get_throttle(&msg);
                altitude = mavlink_msg_vfr_hud_get_alt(&msg); // meters
            return MAVLINK_MSG_ID_VFR_HUD;
            }
            break;
            case MAVLINK_MSG_ID_ATTITUDE:
            {
//                accX = ToDeg(mavlink_msg_attitude_get_pitch(&msg));
//                accY = ToDeg(mavlink_msg_attitude_get_roll(&msg));
//                accZ = ToDeg(mavlink_msg_attitude_get_yaw(&msg));
                return MAVLINK_MSG_ID_ATTITUDE;
            }
            break;
            case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
            {
            //nav_roll = mavlink_msg_nav_controller_output_get_nav_roll(&msg);
            //nav_pitch = mavlink_msg_nav_controller_output_get_nav_pitch(&msg);
            //nav_bearing = mavlink_msg_nav_controller_output_get_nav_bearing(&msg);
               wp_bearing = mavlink_msg_nav_controller_output_get_target_bearing(&msg);
               wp_dist = mavlink_msg_nav_controller_output_get_wp_dist(&msg);
            //alt_error = mavlink_msg_nav_controller_output_get_alt_error(&msg);
            //aspd_error = mavlink_msg_nav_controller_output_get_aspd_error(&msg);
            //xtrack_error = mavlink_msg_nav_controller_output_get_xtrack_error(&msg);
                 return MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT;
            }
            break;
            case MAVLINK_MSG_ID_MISSION_CURRENT:
            {
                 wp_num = (uint8_t)mavlink_msg_mission_current_get_seq(&msg);
                 return MAVLINK_MSG_ID_MISSION_CURRENT;
            }
            break;
        case MAVLINK_MSG_ID_HWSTATUS:  
            {
                 cpu_vcc = mavlink_msg_hwstatus_get_Vcc(&msg) / 100;      // 1 = v/10 0.1v
                 return MAVLINK_MSG_ID_HWSTATUS;
            }
            break;
        case MAVLINK_MSG_ID_STATUSTEXT:
          {   
            last_message_severity = mavlink_msg_statustext_get_severity(&msg);
            mavlink_msg_statustext_get_text(&msg, last_message_text);
			if (last_message_severity >= 3) {
			    status_msg = parse_msg();
				msg_timer = MSG_TIMER;
			}
            return MAVLINK_MSG_ID_STATUSTEXT;
          }
          break;
         default:
            return msg.msgid;
            break;
    }
   }
   // Update global packet drops counter
//    packet_drops += status.packet_rx_drop_count;
//    parse_error += status.parse_error;
   return -1;
}
void Mavlink::printMessage(SoftwareSerial* serialPort, IFrSkyDataProvider* dataProvider, int msg)
{
    if (msg == MAVLINK_MSG_ID_STATUSTEXT)
    {
        serialPort->println("");
        serialPort->print(msg);
        serialPort->print(":");
        serialPort->print(last_message_text);
        serialPort->print(": status_msg=");
        serialPort->println(status_msg);
    } else if (msg == MAVLINK_MSG_ID_HEARTBEAT) {
    } else if (msg == MAVLINK_MSG_ID_SYS_STATUS) {
    } else if (msg == MAVLINK_MSG_ID_GPS_RAW_INT) {
    } else if (msg == MAVLINK_MSG_ID_VFR_HUD) {
    } else if (msg == MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT) {
    } else if (msg == MAVLINK_MSG_ID_ATTITUDE) {
    } else if (msg == MAVLINK_MSG_ID_MISSION_CURRENT) {
    } else if (msg == MAVLINK_MSG_ID_HWSTATUS) {
    } else {
//        serialPort->print("RCV: MAVLINK_MSG_ID ");
//        serialPort->println(msg);
    }
}
void Mavlink::reset()
{
   lastMAVBeat = 0;
// Comment any line for keep last received data
   batteryVoltage = 0;
   current = 0;
   batteryRemaining = 0;
   gpsStatus = 0;
//   latitude = 0;   
//   longitude = 0;
   gpsAltitude = 0;
   gpsHdop = 999;
   numberOfSatelites = 0;
   gpsGroundSpeed = 0;
   gpsCourse = 0;
   altitude = 0.0f;
   alt = 0;
   apmMode = 99; // if no mavlink packets
//   course = 0;
   throttle = 0;
//   accX = 0;
//   accY = 0;
//   accZ = 0;
   last_message_severity = 0;
   status_msg = 0;
   cpu_load = 0;
   cpu_vcc = 0;
//   apmBaseMode = 0; // if system armed, stay armed if link down
   wp_dist = 0;
   wp_num = 0;
   wp_bearing = 270;
   sensors_health = 0;
//   home_gps_alt = 0;
   alt = 0;
   gps_alt = 0;
   lat = 0;
   lon = 0;
}

//------------------ Home Distance and Direction Calculation ----------------------------------
void Mavlink::setHomeVars()
{
    float dstlon, dstlat;
    long bearing;

    if ( isArmed() ){
       if ( gpsStatus > 2 ) {
          gps_alt = (gpsAltitude - home_gps_alt);
	   } else {
	      gps_alt = 0;
	   }
       if (ok) {
          //DST to Home
          lat = latitude / 10000000.0f;
          lon = longitude/ 10000000.0f;
          dstlat = fabs(home_lat - lat) * 111319.5;
          dstlon = fabs(home_lon - lon) * 111319.5 * scaleLongDown;
          home_distance = sqrt(sq(dstlat) + sq(dstlon));
          //DIR to Home
          dstlon = (home_lon - lon); //OffSet_X
          dstlat = (home_lat - lat) * scaleLongUp; //OffSet Y
          bearing = 90 + (atan2(dstlat, -dstlon) * 57.295775); //absolut home direction
          if(bearing < 0) bearing += 360;   //normalization
          bearing = bearing - 180;          //absolut return direction
          if(bearing < 0) bearing += 360;   //normalization
          bearing = bearing - course;       //relative home direction
          if(bearing < 0) bearing += 360;   //normalization
          home_direction = bearing;
       } else {
          home_distance = 0;
          home_direction = 90;
       }
    } else {
	   home_course = course;
       home_direction = 90;
       home_distance = 0;
       gps_alt = gpsAltitude;
       if ( gpsStatus > 2 ) {
          home_gps_alt = gpsAltitude;
          lat = latitude / 10000000.0f;
          lon = longitude/ 10000000.0f;
          home_lat = lat;
          home_lon = lon;
          // shrinking factor for longitude going to poles direction
          rads = fabs(home_lat) * 0.0174532925;
          scaleLongDown = cos(rads);
          scaleLongUp   = 1.0f/cos(rads);
          ok = true;
       } else {
          ok = false;
		  home_gps_alt = 0;
		  lat = 0.0f;
		  lon = 0.0f;
          home_lat = 0.0f;
          home_lon = 0.0f;
	   }
    }
}
boolean Mavlink::getBit(byte Reg, byte whichBit) {
    boolean State;
    State = Reg & (1 << whichBit);
    return State;
}
