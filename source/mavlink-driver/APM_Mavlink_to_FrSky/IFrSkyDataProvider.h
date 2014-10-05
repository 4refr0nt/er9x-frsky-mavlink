/*
	@author 	Nils Hцgberg
	@contact 	nils.hogberg@gmail.com
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
#pragma once
class IFrSkyDataProvider
{
public:
	//IFrSkyDataProvider(void);
	//~IFrSkyDataProvider(void);
	virtual const float	getGpsAltitude() = 0;
	virtual const int	getTemp1() = 0;
	virtual const int	getEngineSpeed() = 0;
	virtual const int	getFuelLevel() = 0;
	virtual const int	getTemp2() = 0;
	virtual const float	getAltitude() = 0;
	virtual const float	getGpsGroundSpeed() = 0;
	virtual const float	getLongitud() = 0;
	virtual const float	getLatitude() = 0;
	virtual const float	getCourse() = 0;
//	virtual const int	getYear() = 0;
//	virtual const int	getDate() = 0;
//	virtual const int	getTime() = 0;
//	virtual const float	getAccX() = 0;
//	virtual const float	getAccY() = 0;
//	virtual const float	getAccZ() = 0;
	virtual const float	getBatteryCurrent() = 0;
	virtual const float	getMainBatteryVoltage() = 0;
	virtual const int   getBaseMode()   = 0;
	virtual const int   getWP_dist()    = 0;
	virtual const int   getWP_num()     = 0;
	virtual const int   getWP_bearing() = 0;
	virtual const int   getHealth()     = 0;
	virtual const int   getStatus_msg() = 0;
	virtual const int   getHome_dir()   = 0;
	virtual const int   getHome_dist()  = 0;
	virtual const int   getCpu_load()   = 0;
	virtual const int   getVcc()        = 0;
	virtual const int   getGpsHdop()    = 0;
	virtual const bool  isArmed()       = 0;
	virtual const int   getNCell()      = 0;
	virtual const int   getCell()       = 0;
};
