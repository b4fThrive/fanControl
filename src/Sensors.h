/* 
 *  Devices classes definitions.
 *  
 *  File: Sensors.h
 *  Author: b4fThrive
 *  Copyright (c) 2020 b4f.thrive@gmail.com
 *  
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *  
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include <iostream>
#include <thread>
#include <vector>

#include "utils.h"

using namespace std;
using namespace utils;

extern const string HDDTEMP_BIN;

/**
 * Sensors abstract class.
 *
 * @class Sensor
 */
class Sensor {
protected:
  string         label;    // System sensor label
  mutable int    minT;     // Minimum working temperature
  mutable int    maxT;     // Maximum working temperature
  mutable int    offsetT;  // Ambient offset temperture, to compare with minimum
  mutable int    temp;     // Sensor temperature input, realtime temperature
  mutable int    tempPerc; // Percentage in temperature range
  string         path;     // Path to device sensor (binary file for hddtemp)
  mutable string cLabel;   // Custom sensor label
  string         devName;  // Device name
  string         name;     // Disk name or file name depending on the type;

  int tempPercentage(int = 0);

public:
  Sensor(string, string, string, string, int = 0, int = 0, int = 0, string = "",
         int = abstract);
  ~Sensor();

  enum sensorTypes { abstract, hwmon, hddtemp };
  int type;

  string getLabel() const;
  int    getMinT() const;
  int    getMaxT() const;
  int    getOffsetT() const;
  int    getTemp() const;
  int    getTempPerc() const;
  string getPath() const;
  string getCLabel() const;
  string getName() const;
  string getDevName() const;

  void setLabel(string);
  void setMinTemp(int);
  void setMaxTemp(int);
  void setoffsetT(int);
  void setTemp(int);
  void setTempPerc(int);
  void setPath(string);
  void setCLabel(string);
  void setName(string);
  void setDevName(string);

  int update(int = 0);

  virtual int readTemp() = 0;
};

typedef vector<Sensor>   sensors_v;
typedef vector<Sensor *> sensors_vp;

/**
 * Fans abstract class.
 *
 * @class Fan
 */
class Fan {
protected:
  int            minS;    // System fan minimum speed
  mutable int    speed;   // Fan input speed
  int            maxS;    // System fan maximum speed
  string         label;   // System fan label
  mutable string cLabel;  // Fan custom label
  string         devName; // Device name

public:
  Fan(string, int, int, string, string = "", int = abstract);
  ~Fan();

  enum fanTypes { abstract, hwmon };
  int type;

  int    getMinS() const;
  int    getMaxS() const;
  string getLabel() const;
  int    getSpeed() const;
  string getCLabel() const;
  string getDevName() const;

  void setMinS(int);
  void setSpeed(int);
  void setMaxS(int);
  void setlabel(string);
  void setCLabel(string);
  void setDevName(string);

  virtual void   manualModeOn()   = 0; // Turns on fan controller
  virtual void   manualModeOff()  = 0; // Turns off fan controller
  virtual int    readSpeed()      = 0; // Reads fan speed
  virtual void   changeSpeed(int) = 0; // Changes fan speed
  virtual string getPath() const  = 0; // Gets device path
  virtual string getName() const  = 0; // Gets fan file path
};

/**
 * hwmon Sensor class.
 *
 * @class HwMonSensor : public Sensor
 */
class HwMonSensor : public Sensor {
private:
  string pInput; // Input sensor file path

public:
  HwMonSensor(string, string, string, int = 45, int = 78, int = 24, string = "",
              string = "");
  ~HwMonSensor();

  string getInputPath() const;

  int readTemp();
};

typedef vector<HwMonSensor>   hwmSens_v;
typedef vector<HwMonSensor *> hwmSens_vp;

/**
 * hddtemp Sensor class.
 *
 * @class HddTempSensor : public Sensor
 */
class HddTempSensor : public Sensor {
private:
  string cInput; // Input sensor command

public:
  HddTempSensor(string, int = 45, int = 63, int = 27, string = "", string = "");
  ~HddTempSensor();

  int readTemp();
};

typedef vector<HddTempSensor>   hddtSens_v;
typedef vector<HddTempSensor *> hddtSens_vp;

/**
 * hwmon Fan class.
 *
 * @class HwMonSensor : public Fan
 */
class HwMonFan : public Fan {
private:
  string       path;        // hwmon path
  string       fileName;    // hwmon fan file name without sufix
  string       pInput;      // Input speed fan file path
  string       pOutput;     // Output speed fan file path
  string       pManual;     // Manual selector fan file path
  mutable bool manModeStat; // Manual mode status

  void manualMode(bool);

public:
  HwMonFan(string, string, string, string = "");
  ~HwMonFan();

  string getPath() const;
  string getName() const;

  void manualModeOn();
  void manualModeOff();

  int readSpeed();

  void changeSpeed(int);
};

typedef vector<HwMonFan>   fans_v;
typedef vector<HwMonFan *> fans_vp;

/**
 * Generic fan node class, it can be any derived from Fan abstract class.
 * Is an association between a fan an their sensors
 *
 * . WARNING: YOU MUST MANUALLY DELETE THE POINTERS IF NEEDED
 * . YOU HAVE ::clearSensors() function helper TO DO IT.
 *
 * @class FanNode
 */
class FanNode {
private:
  mutable Fan *       fan;     // hwmon fan
  mutable sensors_vp *sensors; // Associated sensors

public:
  FanNode(Fan *, sensors_vp * = new sensors_vp);
  ~FanNode();

  Fan *       getFan() const;
  sensors_vp *getSensors() const;

  void setFan(Fan *);
  void setSensors(sensors_vp *, bool = false);

  void pushBackSensor(Sensor *);
  void popBackSensor();

  void clearSensors();

  int  getMaxPercentage(int);
  void update(int);
};

typedef vector<FanNode>   fanNode_v;
typedef vector<FanNode *> fanNode_vp;

/**
 * Fans controller class. It controls fan nodes.
 *
 * . WARNING: YOU MUST MANUALLY DELETE THE POINTERS IF NEEDED
 * . YOU HAVE ::clearFans() and ::clearAll() functions helper TO DO IT.
 *
 * @class FanController
 */
class FanController {
private:
  Sensor *            ambSensor;
  mutable fanNode_vp *fans;

  mutable bool    working; // Worker is working control
  mutable thread *worker;  // Worker thread

  static void threadLoop(FanController *);

public:
  FanController(fanNode_vp * = new fanNode_vp, Sensor * = nullptr);
  FanController(Sensor *, fanNode_vp * = new fanNode_vp);
  FanController(FanController *);
  ~FanController();

  Sensor *    getAmbSensor() const;
  fanNode_vp *getFans() const;

  thread *getWorker();

  void setAmbSensor(Sensor * = nullptr, bool = true);
  void setFans(fanNode_vp * = nullptr, bool = true);

  void pushBackFanNode(FanNode *);
  void popBackFanNode();

  void startWorker();
  void stopWorker();

  void clearFans();
  void clearAll();
};

/**
 * hddtemp device struct. Contains disk info.
 *
 * @struct Disks
 */
struct Disks {
  string disk;   // disk name
  string device; // disk device path
  string model;  // disk model
  string serial; // disk serial

  Disks(string);
  ~Disks();
};

typedef vector<Disks>   disks_v;
typedef vector<Disks *> disks_vp;

/**
 * hwmon device struct. Contains fans and sensors from hwmon driver device
 *
 * @struct HwmonDevice
 */
struct HwmonDevice {
  string     name;    // Device name
  string     path;    // Device full path
  fans_vp    fans;    // Device fans
  hwmSens_vp sensors; // Device sensors

  HwmonDevice(string);
  ~HwmonDevice();
};

typedef vector<HwmonDevice>   hwmonDevs_v;
typedef vector<HwmonDevice *> hwmonDevs_vp;

/**
 * System devices struct.
 *
 * @struct SystemDevices
 */
struct SystemDevices {
  disks_vp     disks;          // Disks
  hwmonDevs_vp hwmonDevices;   // hwmon devices
  hddtSens_vp  hddtempSensors; // hddtemp sensors

  unsigned int nFans;      // Number of fans
  unsigned int nSensors;   // Number of sensors
  unsigned int nHwmonDevs; // number of hwmon devices
  unsigned int nDisks;     // number of disks

  SystemDevices();
  ~SystemDevices();
};

#endif /* SENSORS_H_ */
