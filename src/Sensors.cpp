#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <thread>
#include <vector>

#include "Sensors.h"
#include "shell_commands.h"
#include "utils.h"

using namespace std;
using namespace utils;
using namespace utils;

const char *HDDTEMP_BIN = utils::ShellCommand(HDDTEMP_PATH).firsLine().c_str();

Sensor::Sensor(string devName, string path, string name, string label, int minT,
               int maxT, int offsetT, string cLabel, int type)
    : devName(devName), path(path), name(name), label(label),
      minT(minT < 1000 ? minT * 1000 : minT),
      maxT(maxT < 1000 ? maxT * 1000 : maxT),
      offsetT(offsetT < 1000 ? offsetT * 1000 : offsetT), cLabel(cLabel),
      type(type) {}
Sensor::~Sensor() {}

string Sensor::getLabel() const { return label; }
int    Sensor::getMinT() const { return minT; }
int    Sensor::getMaxT() const { return maxT; }
int    Sensor::getOffsetT() const { return offsetT; }
int    Sensor::getTemp() const { return temp; }
int    Sensor::getTempPerc() const { return tempPerc; }
string Sensor::getPath() const { return path; }
string Sensor::getCLabel() const { return cLabel; }
string Sensor::getName() const { return name; }
string Sensor::getDevName() const { return devName; }

void Sensor::setLabel(string _label) { label = _label; }
void Sensor::setMinTemp(int _minT) {
  minT = _minT < 1000 ? _minT * 1000 : _minT;
}
void Sensor::setMaxTemp(int _maxT) {
  maxT = _maxT < 1000 ? _maxT * 1000 : _maxT;
}
void Sensor::setoffsetT(int _offsetT) {
  offsetT = _offsetT < 1000 ? _offsetT * 1000 : _offsetT;
}
void Sensor::setTemp(int _temp) { temp = _temp; }
void Sensor::setTempPerc(int _tempPerc) { tempPerc = _tempPerc; }
void Sensor::setPath(string _path) { path = _path; }
void Sensor::setCLabel(string _cLabel) { cLabel = _cLabel; }
void Sensor::setName(string _name) { name = _name; }
void Sensor::setDevName(string _devName) { devName = _devName; }

int Sensor::update(int ambT) { return tempPerc = tempPercentage(ambT); }

/**
 * Sensors abstract class.
 *
 * @class Sensor
 * @protected Sensor::tempPercentage
 *
 * Calcule percentage on the range of temperatures
 *
 * @param  {int} ambT   : Ambient temperature
 *
 * @return {int}        : Percentage on the range
 */
int Sensor::tempPercentage(int ambT) {
  if (readTemp() >= maxT) return 100;

  int minTemp = max(ambT + offsetT, minT);

  // Reconfigure minimum temp with high ambient temperature
  if (minTemp >= maxT) minTemp = maxT - 3000;

  return temp > minT ? ((temp - minTemp) * 100) / (maxT - minTemp) : 0;
}

Fan::Fan(string devName, int min, int max, string label, string cLabel,
         int type)
    : devName(devName), minS(min), speed(0), maxS(max), label(label),
      cLabel(cLabel), type(type) {}
Fan::~Fan() {}

int    Fan::getMinS() const { return minS; }
int    Fan::getMaxS() const { return maxS; }
string Fan::getLabel() const { return label; }
int    Fan::getSpeed() const { return speed; }
string Fan::getCLabel() const { return cLabel; }
string Fan::getDevName() const { return devName; }

void Fan::setMinS(int _minS) { minS = _minS; }
void Fan::setSpeed(int _speed) { speed = _speed; }
void Fan::setMaxS(int _maxS) { maxS = _maxS; }
void Fan::setlabel(string _label) { label = _label; }
void Fan::setCLabel(string _cLabel) { cLabel = _cLabel; }
void Fan::setDevName(string _devName) { devName = _devName; }

/**
 * hwmon Sensor class constructor.
 *
 * @class  HwMonSensor : public Sensor
 * @public HwMonSensor::HwMonSensor
 *
 * @param  {string} devName : hwmon device name (ex: applesmc)
 * @param  {string} path    : hwmon directory path
 * @param  {string} name    : Sensor file name without sufix (_label|_input)
 * @param  {int} minT       : Minimum working temperature
 * @param  {int} maxT       : Maximum working temperature
 * @param  {int} offsetT    : Offset temperatur
 * @param  {string} cLabel  : Custo label
 */
HwMonSensor::HwMonSensor(string devName, string path, string name, int minT,
                         int maxT, int offsetT, string label, string cLabel)
    : Sensor(devName, checkDir(path), name, label, minT, maxT, offsetT, cLabel,
             hwmon),
      pInput(path + name + "_input") {
  if (label == "") {
    ShellCommand shell;
    if (shell.exec(CAT(path + name + "_label"))) {
      label = shell.firsLine();
      while (label[label.size() - 1] == ' ')
        label = label.substr(0, label.size() - 1);
      setLabel(label);
    } else
      setLabel(name);
  }
  if (cLabel == "") setCLabel(devName + "_" + getLabel());
  readTemp();
}
HwMonSensor::~HwMonSensor() {}

string HwMonSensor::getInputPath() const { return pInput; }

int HwMonSensor::readTemp() { return temp = stoi(readFile(pInput)); }

/**
 * hddtemp Sensor class constructor.
 *
 * @class  HddTempSensor : public Sensor
 * @public HddTempSensor::HddTempSensor
 *
 * @param  {string} name    : Disk name
 * @param  {int} minT       : Minimum working temperature
 * @param  {int} maxT       : Maximum working temperature
 * @param  {int} offsetT    : Offset temperatur
 * @param  {string} cLabel  : Custo label
 */
HddTempSensor::HddTempSensor(string name, int minT, int maxT, int offsetT,
                             string cLabel, string path)
    : Sensor("hddTemp", path == "" ? HDDTEMP_BIN : path, name,
             ShellCommand(DISK_MODEL(name)).firsLine(), minT, maxT, offsetT,
             cLabel, hddtemp),
      cInput(HDDTEMP_GET(name)) {
  if (cLabel == "") setCLabel(devName + "_" + label);
  readTemp();
}
HddTempSensor::~HddTempSensor() {}

/**
 * hddtemp Sensor class function.
 *
 * @class  HddTempSensor : public Sensor
 * @public HddTempSensor::readTemp
 *
 * @return {int} : Current temperature
 */
int HddTempSensor::readTemp() {
  return temp = stoi("0" + ShellCommand(cInput).firsLine()) * 1000;
}

/**
 * hwmon Fan class constructor.
 *
 * @class  HwMonSensor : public Fan
 * @public HwMonFan::HwMonFan
 *
 * @param  {string} devName  : hwmon device name (ex: applesmc)
 * @param  {string} path     : Directory path
 * @param  {string} fileName : Fan file name without suffix (_label|_input...)
 * @param  {string} cLabel   : Fan custom label
 */
HwMonFan::HwMonFan(string devName, string path, string fileName, string cLabel)
    : Fan(devName, stoi(readFile(checkDir(path) + fileName + "_min")),
          stoi(readFile(path + fileName + "_max")),
          readFile(path + fileName + "_label"), cLabel, hwmon),
      path(path), fileName(fileName), pInput(path + fileName + "_input"),
      pOutput(path + fileName + "_output"),
      pManual(path + fileName + "_manual"), manModeStat(false) {
  while (label[label.size() - 1] == ' ')
    label = label.substr(0, label.size() - 1);
  if (cLabel == "") setCLabel(label + " " + devName);
}

HwMonFan::~HwMonFan() { manualModeOff(); }

/**
 * hwmon Fan class function.
 *
 * @class   HwMonSensor : public Fan
 * @private HwMonFan::manualMode
 *
 * @param  {bool} mode : On off switcher
 */
void HwMonFan::manualMode(bool mode) {
  if (mode != manModeStat) {
    writeFile(pManual, mode ? "1" : "0");
    manModeStat = mode;
  }
}

string HwMonFan::getPath() const { return path; }
string HwMonFan::getName() const { return fileName; }

void HwMonFan::manualModeOn() { manualMode(true); }
void HwMonFan::manualModeOff() { manualMode(false); }

int HwMonFan::readSpeed() { return stoi(readFile(pInput)); }

/**
 * Changes current fan speed
 *
 * @class  HwMonSensor : public Fan
 * @public HwMonFan::changeSpeed
 *
 * @param  {int} newSpeed : New fan speed
 */
void HwMonFan::changeSpeed(int newSpeed) {
  if (manModeStat && newSpeed != speed) {
    writeFile(pOutput, to_string(newSpeed));
    setSpeed(newSpeed);
  }
}

FanNode::FanNode(Fan *fan, sensors_vp *sens) : fan(fan), sensors(sens) {}
FanNode::~FanNode() {}

Fan *       FanNode::getFan() const { return fan; }
sensors_vp *FanNode::getSensors() const { return sensors; }

void FanNode::setFan(Fan *_fan) { fan = _fan; }
void FanNode::setSensors(sensors_vp *_sensors, bool clearBefore) {
  if (clearBefore) clearSensors();
  delete sensors;
  sensors = _sensors;
}

void FanNode::pushBackSensor(Sensor *sensor) { sensors->push_back(sensor); }
void FanNode::popBackSensor() { sensors->pop_back(); }

/**
 * Generic fan node class function. Deletes all sensors
 *
 * @class  FanNode
 * @public FanNode::clearSensors
 */
void FanNode::clearSensors() {
  int senSize = sensors->size();

  if (senSize > 0)
    for (int i = 0; i < senSize; i++) {
      delete (*sensors)[i];
      (*sensors)[i] = nullptr;
    }

  sensors->clear();
}

/**
 * Generic fan node class function.
 *
 * @class  FanNode
 * @public FanNode::getMaxPercentage
 *
 * @param  {int} ambT : Ambient temperature
 * @return {int}      : Maximum percentage range from all sensors
 */
int FanNode::getMaxPercentage(int ambT) {
  int senSize = sensors->size();
  int maxPerc = 0;

  if (senSize > 0)
    for (int i = 0; i < senSize; i++)
      maxPerc = max((*sensors)[i]->update(ambT), maxPerc);

  return maxPerc;
}

/**
 * Generic fan node class function. Updates the fan speed
 *
 * @class  FanNode
 * @public FanNode::update
 *
 * @param  {int} ambT : Ambient temperature
 */
void FanNode::update(int ambT) {
  int minS  = fan->getMinS();
  int maxS  = fan->getMaxS();
  int maxP  = getMaxPercentage(ambT);
  int speed = minS + ((maxS - minS) * maxP / 100);

  if (speed != fan->getSpeed()) fan->changeSpeed(speed);
}

FanController::FanController(fanNode_vp *fans, Sensor *ambSensor)
    : ambSensor(nullptr), fans(!fans ? new fanNode_vp : fans), working(false),
      worker(nullptr) {}
FanController::FanController(Sensor *ambSensor, fanNode_vp *fans)
    : ambSensor(ambSensor), fans(!fans ? new fanNode_vp : fans), working(false),
      worker(nullptr) {}
FanController::FanController(FanController *fanCtl)
    : ambSensor(fanCtl->getAmbSensor()), fans(fanCtl->getFans()),
      working(false), worker(nullptr) {}

/**
 * Fans controller class destructor.
 *
 * @class FanController
 * @public FanController::~FanController
 */
FanController::~FanController() {
  if (worker || working) {
    stopWorker();
    delete worker;
    worker = nullptr;
  }
}

/**
 * Fans controller class static function. Thread worker loop.
 *
 * @class   FanController
 * @private FanController::threadLoop
 *
 * @param  {FanController*} _this : Pointer FanController
 */
void FanController::threadLoop(FanController *_this) {
  fanNode_vp *fans = _this->fans;
  int         fansSize;

  while (_this && _this->working && (fansSize = fans->size()) > 0) {
    int ambT = !_this->ambSensor ? 0 : _this->ambSensor->readTemp();
    for (unsigned int i = 0; i < fansSize; i++) (*fans)[i]->update(ambT);
    this_thread::sleep_for(chrono::seconds(1));
  }
}

Sensor *    FanController::getAmbSensor() const { return ambSensor; }
fanNode_vp *FanController::getFans() const { return fans; }

thread *FanController::getWorker() { return worker; }

void FanController::setAmbSensor(Sensor *_ambSensor, bool delBefore) {
  if (delBefore && ambSensor) delete ambSensor;
  ambSensor = _ambSensor;
}
void FanController::setFans(fanNode_vp *_fans, bool delBefore) {
  if (delBefore && fans) delete fans;
  fans = _fans;
}

void FanController::pushBackFanNode(FanNode *node) { fans->push_back(node); }
void FanController::popBackFanNode() { fans->pop_back(); }

/**
 * Fans controller class function.
 * Starts the worker wich controlls fans speeds.
 *
 * @class  FanController
 * @public FanController::startWorker
 */
void FanController::startWorker() {
  int fansSize = fans->size();

  if (!working && fansSize > 0 && !worker) {
    for (unsigned int i = 0; i < fansSize; i++)
      fans->at(i)->getFan()->manualModeOn();

    working = true;
    worker  = new thread(threadLoop, this);
  }
}

/**
 * Fans controller class function.
 * Stops the worker wich controlls fans speeds.
 *
 * @class  FanController
 * @public FanController::stopWorker
 */
void FanController::stopWorker() {
  if (working || worker) {
    working = false;

    if (worker) {
      worker->join();
      delete worker;
      worker = nullptr;
    }

    int fansSize = fans->size();
    for (int i = 0; i < fansSize; i++) fans->at(i)->getFan()->manualModeOff();
  }
}

void FanController::clearFans() {
  int fansSize = fans->size();

  for (int i = 0; i < fans->size(); i++) {
    (*fans)[i]->clearSensors();
    delete (*fans)[i]->getFan();
    (*fans)[i]->setFan(nullptr);
    delete (*fans)[i];
    (*fans)[i] = nullptr;
    fans->clear();
  }
}

void FanController::clearAll() {
  if (fans) clearFans();
  if (ambSensor) delete ambSensor;
  ambSensor = nullptr;
}

/**
 * hddtemp device struct constructor.
 *
 * @struct Disks
 * @public Disks::Disks
 *
 * @param  {string} disk : Disk name. Example: sda | sdb | hda...
 */
Disks::Disks(string disk) : disk(disk) {
  ShellCommand shell(DISK_INFO_PF + disk + DISK_INFO_SF);

  if (shell.getSize() < 2) throw runtime_error("Can't read disk info");

  device = "/dev" + disk;
  serial = shell.firsLine();
  model  = shell.lastLine();
}

Disks::~Disks() {}

/**
 * hwmon device struct constructor.
 *
 * @struct HwmonDevice
 * @public HwmonDevice::HwmonDevice
 *
 * @param  {string} path : hwmon device driver path
 */
HwmonDevice::HwmonDevice(string path) : path(checkDir(path)) {
  ShellCommand shell;
  string       lineComm = "";

  if (!shell.exec(CAT(path + "name"))) {
    path += "device/";
    if (!shell.exec(CAT(path + "name")))
      throw runtime_error("Cannot find hwmon device on '" + path + "'");
  }

  name = shell.firsLine();

  if (shell.exec(LS(path + "fan*_input") " | sed 's,_input,,g'")) {
    shell.getLine(lineComm);

    int    startPos = lineComm.find_last_of('/') + 1;
    string fanPath  = lineComm.substr(0, startPos);

    do {
      string    fanName = lineComm.substr(startPos);
      HwMonFan *fan     = new HwMonFan(name, fanPath, fanName);
      fans.push_back(fan);
    } while (shell.getLine(lineComm));
  }

  if (shell.exec(LS(path + "temp*_input") " | sed 's,_input,,g'")) {
    shell.getLine(lineComm);

    int    startPos   = lineComm.find_last_of('/') + 1;
    string sensorPath = lineComm.substr(0, startPos - 1);

    do {
      string       sensorName = lineComm.substr(startPos);
      HwMonSensor *sensor     = new HwMonSensor(name, sensorPath, sensorName);
      sensors.push_back(sensor);
    } while (shell.getLine(lineComm));
  }
}

HwmonDevice::~HwmonDevice() {
  int fansSize    = fans.size();
  int sensorsSize = sensors.size();
  int maxSize     = fansSize > sensorsSize ? fansSize : sensorsSize;

  for (int i = 0; i < maxSize; i++) {
    if (i < fansSize) {
      delete fans[i];
      fans[i] = nullptr;
    }

    if (i < sensorsSize) {
      delete sensors[i];
      sensors[i] = nullptr;
    }
  }
}

/**
 * System devices struct constructor.
 *
 * @struct SystemDevices
 * @public SystemDevices::~SystemDevices
 */
SystemDevices::SystemDevices() : nFans(0), nSensors(0) {
  ShellCommand shell;

  if (!shell.exec(HWMON_DEVS)) throw runtime_error("hwmon devices not found");

  string hwmonPath;
  while (shell.getLine(hwmonPath)) {
    HwmonDevice *hwmonDev = new HwmonDevice(hwmonPath);
    hwmonDevices.push_back(hwmonDev);

    nFans += hwmonDev->fans.size();
    nSensors += hwmonDev->sensors.size();
  }

  if (shell.exec(HDDTEMP_PATH)) {
    string hddtempPath = shell.firsLine();

    if (!shell.exec(DISKS)) throw runtime_error("Error reading disks devices");

    string disk;
    while (shell.getLine(disk)) {
      Disks *        hddDev  = new Disks(disk);
      HddTempSensor *hddtemp = new HddTempSensor(
          hddDev->disk, 45, 63, 27, hddDev->model, hddtempPath);
      disks.push_back(hddDev);
      hddtempSensors.push_back(hddtemp);
    }
  }

  nHwmonDevs = hwmonDevices.size();
  nDisks     = disks.size();
}

SystemDevices::~SystemDevices() {
  int maxSize = max(nHwmonDevs, nDisks);

  for (int i = 0; i < maxSize; i++) {
    if (i < nHwmonDevs) {
      delete hwmonDevices[i];
      hwmonDevices[i] = nullptr;
    }

    if (i < nDisks) {
      delete disks[i];
      delete hddtempSensors[i];
      disks[i]          = nullptr;
      hddtempSensors[i] = nullptr;
    }
  }

  hwmonDevices.clear();
  hddtempSensors.clear();
  disks.clear();
}
