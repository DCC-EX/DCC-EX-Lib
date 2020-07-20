/*
 *  WiThrottle.h
 * 
 *  This file is part of CommandStation.
 *
 *  CommandStation is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CommandStation is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMMANDSTATION_COMMINTERFACE_WIFI_WITHROTTLE_H_
#define COMMANDSTATION_COMMINTERFACE_WIFI_WITHROTTLE_H_

struct MYLOCO
{
  char throttle;
  int cab;
};

class WiThrottle
{
public:
  static void loop();
  void parse(Print &stream, char *cmd);
  static WiThrottle *getThrottle(Print &stream, int wifiClient);

private:
  WiThrottle(Print &stream, int comId,int wifiClientId);
  ~WiThrottle();

  static const int MAX_MY_LOCO = 10;
  static const int HEARTBEAT_TIMEOUT = 10;
  static WiThrottle *firstThrottle;
  static int getInt(char *cmd);
  static int getLocoId(char *cmd);

  WiThrottle *nextThrottle;
  int clientid;
  int comId;
  
  MYLOCO myLocos[MAX_MY_LOCO];
  bool heartBeatEnable;
  unsigned long heartBeat;

  void multithrottle(Print &stream, char *cmd);
  void locoAction(Print &stream, char *aval, char throttleChar, int cab);
  void accessory(Print &stream, char *cmd);
  void checkHeartbeat();
};

#endif  // COMMANDSTATION_COMMINTERFACE_WIFI_WITHROTTLE_H_
