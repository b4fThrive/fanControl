/* 
 *  Fan control main file
 *  This app is a wrapper to control for hwmon fans and sensors driver
 *  It controls fan speeds based on preconfigured temperatures ranges
 *  
 *  File: main.cpp
 *  Author: b4fThrive
 *  Copyright (c) 2020 2020 b4f.thrive@gmail.com
 *  
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *  
 */

#include "main.h"
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "Sensors.h"
#include "config_menu.h"
#include "fanControlConfig.h"
#include "main.h"
#include "shell_commands.h"
#include "utils.h"

using namespace std;

/******************************************************************************
 * Aplication globals
 ******************************************************************************/

const string APP_USER  = getenv("USER");
const string HOME_PATH = getenv("HOME");
const string APP_PATH  = HOME_PATH + "/.fanControl";
const string CFG_FILE  = APP_PATH + "/config";
const string LOG_FILE  = APP_PATH + "/log";
const string CRASH_LOG = APP_PATH + "/crashlog";
const string VAR_DIR   = "/var/run/fanControl";
const string PID_FILE  = VAR_DIR + "/pid";
const string USR_FILE  = VAR_DIR + "/usr";

FanController *fanController = nullptr; // FanController used for the service

/******************************************************************************
 * Aplication commands
 ******************************************************************************/

const int    COM_SIZE           = 7;
const string COM_LIST[COM_SIZE] = {
    "start", "stop", "restart", "status", "config", "help", "version"};

/******************************************************************************
 * Main
 ******************************************************************************/

int main(int argc, char const *argv[]) {
  int argument = argc == 1 ? noComm : command(string(argv[1]));

  if (argument <= noComm) // error at argument readed
  {
    cout << "Error: "
         << (argument == noComm ? "Needed a command." : "Unknow command")
         << endl
         << endl;
    showHelp();

    exit(EXIT_FAILURE);
  }

  switch (argument) { // clang-format off
    case restart: stopApp();
    case start  : startApp();     break;
    case stop   : stopApp();      break;
    case help   : showHelp();     break;
    case config : configWizard(); break;
    case status : appStatus();    break;
    case version: showVers();     break;
  } // clang-format on

  exit(EXIT_SUCCESS);
}

/******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @param  {string} arg : Application argument
 * @return {int}        : Command number from enum list
 */
int command(string arg) {
  for (int i = 0; i < COM_SIZE; i++)
    if (arg == COM_LIST[i]) return i;

  return eComm;
}

// Starts fanControl service
void startApp() {
  ShComm shell;

  umask(007);

  if (!shell.exec(LS_LA(APP_PATH))) mkdir(APP_PATH.c_str(), 0770);

  // check if fanControl is already running
  if (shell.exec(CAT(PID_FILE)) && shell.exec(CAT(USR_FILE))) {
    string user = shell.firsLine();
    string eMsg = APP_USER == user
                      ? "fanControl is already running"
                      : "The user " + user + " has fanControl already running";
    crashLog(eMsg);
    cout << eMsg << endl;
    exit(EXIT_FAILURE);
  }

  if (!shell.exec(LS(CFG_FILE))) {
    string eMsg = "Cannot read fanControl config '" + CFG_FILE + "'";
    crashLog(eMsg);
    cout << eMsg << endl;
    exit(EXIT_FAILURE);
  }

  readConfig(fanController);

  pid_t sid, pid = fork();

  if (pid < 0) // chesk start child process
  {
    string eMsg = "Cannot start fanControl, forks fails";
    crashLog(eMsg);
    cout << eMsg << endl;
    exit(EXIT_FAILURE);
  }

  if (pid > 0) // exits parent process
  {
    int pidNum = 0, i = 0;

    cout << "Starting fanControl" << endl;

    while (!shell.exec(CAT(PID_FILE)) && ++i < 36) {
      cout.flush();
      cout << ".";
      this_thread::sleep_for(chrono::milliseconds(250));
    }

    string PID = shell.firsLine();

    if (PID == "") {
      string eMsg = "Cannot start fanControl.";

      crashLog(eMsg);
      cout << endl << eMsg << endl;

      exit(EXIT_FAILURE);
    }

    appLog("fanControl started with pid " + PID);
    cout << endl << "fanControl started with pid " + PID << endl;

    exit(EXIT_SUCCESS);
  }

  sid = setsid();

  if (!shell.exec(LS_LA(VAR_DIR))) mkdir(VAR_DIR.c_str(), 0770);

  if (!writeFile(USR_FILE, APP_USER)) {
    string eMsg = "Error writting file " + USR_FILE;
    crashLog(eMsg);
    cout << eMsg << endl;
    exit(EXIT_FAILURE);
  }
  if (!writeFile(PID_FILE, to_string(sid))) {
    string eMsg = "Error writting file " + PID_FILE;
    crashLog(eMsg);
    cout << eMsg << endl;
    exit(EXIT_FAILURE);
  }

  fanController->startWorker();

  closeSTDdescriptors();

  // SIGTERM capture to stop the worker if needed
  signal(SIGTERM, signHandler);

  if (fanController && fanController->getWorker())
    fanController->getWorker()->join();
}

// Stops fanControl service
void stopApp() {
  ShComm shell;
  string pidRun = readFile(PID_FILE, true);

  // checks if fanControl is running
  if (pidRun == "") {
    cout << "fanControl is not running" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Stopping fanControl" << endl;

  if (system(("kill " + pidRun).c_str()) < 0) // kill child process
  {
    string eMsg = "Cannot stop fanControl with pid=" + pidRun;
    crashLog(eMsg);
    cout << endl
         << eMsg << endl
         << "Try again or manually kill the process." << endl;
    exit(EXIT_FAILURE);
  }

  int  i       = 0;
  bool running = shell.exec(LS(("/proc/" + pidRun)));
  while ((running = shell.exec()) && ++i < 36) {
    cout.flush();
    cout << ".";
    this_thread::sleep_for(chrono::milliseconds(250));
  }

  if (running) {
    string eMsg = "Cannot stop fanControl with pid=" + pidRun +
                  ", exceded maximum time to stop it.";
    crashLog(eMsg);
    cout << endl
         << eMsg << endl
         << "Try again or manually kill the process." << endl;
    exit(EXIT_FAILURE);
  }

  if (system("rm /var/run/fanControl/*") < 0) { // remove var files
    string eMsg = "Cannot delete fanControl /var/run files";
    crashLog(eMsg);
    cout << endl
         << eMsg << endl
         << "Please delete the files with the command "
            "'rm /var/run/fanControl/*' "
            "if you want start fanControl before reboot "
            "or shutdown the system."
         << endl;
  }

  cout << endl << "fanControl stopped" << endl;
}

// Starts the config wizard
void configWizard() {
  ShComm shell;

  umask(007);

  if (!shell.exec(LS_LA(APP_PATH))) mkdir(APP_PATH.c_str(), 0770);

  string title = "###############################"
                 " FanControl Configuration Mode "
                 "###############################";
  ConfigMode cfgWizard(title);

  cfgWizard.start();
}

// Show service status
void appStatus() {
  ShComm shell;

  if (shell.exec(CAT(PID_FILE))) {
    string pid = shell.firsLine();
    shell.exec(CAT(USR_FILE));
    string user = shell.firsLine();

    cout << user << " is running an instance of fanControl with pid: " << pid
         << endl;
  } else
    cout << "FanControl is not running" << endl;
}

// Show help commands
void showHelp() {
  cout << "Usage: fanControl <command>" << endl
       << endl
       << "Commands:" << endl
       << "      start     Starts fanControl" << endl
       << "      stop      Stops fanControl" << endl
       << "      restart   Restarts fanControl" << endl
       << "      config    Configuration wizard" << endl
       << "      status    Show fanControl status" << endl
       << "      help      Show this help" << endl
       << endl;
}

// Show app version
void showVers() {
  cout << "fanControl version " << fanControl_VERSION_MAJOR << "."
       << fanControl_VERSION_MINOR << "." << fanControl_VERSION_PATCH << endl;
}

// Reads config file
void readConfig(FanController *&fanCtl) {
  ifstream    configFile(CFG_FILE);
  fanNode_vp *fans = new fanNode_vp;
  int         fansSize;
  Sensor *    ambSensor = nullptr;
  string      fansSizeStr, ambSensorType, ambSensorPath, ambSensorDevName,
      ambSensorName, ambSensorMin, ambSensorMax, ambSensorOffset,
      ambSensorCLabel;

  if (!configFile.is_open())
    throw runtime_error("Error opening config file " + CFG_FILE);

  getline(configFile, fansSizeStr);
  fansSize = stoi(fansSizeStr);

  for (int i = 0; i < fansSize; i++) {
    Fan *             fan     = nullptr;
    vector<Sensor *> *sensors = new vector<Sensor *>;
    string fanType, fanDevName, fanPath, fanName, fanCLabel, sensorsSize;

    getline(configFile, fanType);
    getline(configFile, fanDevName);
    getline(configFile, fanPath);
    getline(configFile, fanName);
    getline(configFile, fanCLabel);
    getline(configFile, sensorsSize);

    if (stoi(fanType) == Fan::hwmon)
      fan = new HwMonFan(fanDevName, fanPath, fanName, fanCLabel);

    for (int i = 0; i < stoi(sensorsSize); i++) {
      string type, sensorDevName, sensorPath, sensorName, minT, maxT, offsetT,
          sensorCLabel;

      getline(configFile, type);
      getline(configFile, sensorDevName);
      getline(configFile, sensorPath);
      getline(configFile, sensorName);
      getline(configFile, minT);
      getline(configFile, maxT);
      getline(configFile, offsetT);
      getline(configFile, sensorCLabel);

      Sensor *sensor = nullptr;

      if (stoi(type) == Sensor::hwmon)
        sensor = new HwMonSensor(sensorDevName,
                                 sensorPath,
                                 sensorName,
                                 stoi(minT),
                                 stoi(maxT),
                                 stoi(offsetT),
                                 "",
                                 sensorCLabel);
      else
        sensor = new HddTempSensor(sensorName,
                                   stoi(minT),
                                   stoi(maxT),
                                   stoi(offsetT),
                                   sensorCLabel,
                                   sensorPath);

      sensors->push_back(sensor);
    }
    fans->push_back(new FanNode(fan, sensors));
  }

  getline(configFile, ambSensorType);
  getline(configFile, ambSensorDevName);
  getline(configFile, ambSensorPath);
  getline(configFile, ambSensorName);
  getline(configFile, ambSensorMin);
  getline(configFile, ambSensorMax);
  getline(configFile, ambSensorOffset);
  getline(configFile, ambSensorCLabel);

  configFile.close();

  // clang-format off
  ambSensor = new HwMonSensor(ambSensorDevName,ambSensorPath, ambSensorName, 
                              stoi(ambSensorMin), stoi(ambSensorMax), 
                              stoi(ambSensorOffset), "", ambSensorCLabel);
  // clang-format on

  if (fanCtl) {
    delete fanCtl;
    fanCtl = nullptr;
  }

  fanCtl = new FanController(ambSensor, fans);
}

// Writes config file
void writeConfig(FanController *fanCtl) {
  ofstream   configFile(CFG_FILE);
  fanNode_vp fans      = *fanCtl->getFans();
  int        fansSize  = fans.size();
  Sensor *   ambSensor = fanCtl->getAmbSensor();

  umask(007);

  if (!configFile.is_open())
    throw runtime_error("Error opening config file " + CFG_FILE);

  configFile << fansSize << endl;

  for (int i = 0; i < fansSize; i++) {
    Fan *             fan         = fans[i]->getFan();
    vector<Sensor *> *sensors     = fans[i]->getSensors();
    int               sensorsSize = sensors->size();

    configFile << fan->type << endl
               << fan->getDevName() << endl
               << fan->getPath() << endl
               << fan->getName() << endl
               << fan->getCLabel() << endl
               << sensorsSize << endl;

    for (int i = 0; i < sensorsSize; i++) {
      Sensor *sensor = (*sensors)[i];

      configFile << sensor->type << endl
                 << sensor->getDevName() << endl
                 << sensor->getPath() << endl
                 << sensor->getName() << endl
                 << sensor->getMinT() / 1000 << endl
                 << sensor->getMaxT() / 1000 << endl
                 << sensor->getOffsetT() / 1000 << endl
                 << sensor->getCLabel() << endl;
    }
  }

  configFile << ambSensor->type << endl
             << ambSensor->getDevName() << endl
             << ambSensor->getPath() << endl
             << ambSensor->getName() << endl
             << ambSensor->getMinT() / 1000 << endl
             << ambSensor->getMaxT() / 1000 << endl
             << ambSensor->getOffsetT() / 1000 << endl
             << ambSensor->getCLabel() << endl;

  configFile.close();
}

string logMsg(string msg) {
  time_t currentTime = time(NULL);
  tm *   localTime   = localtime(&currentTime);
  char   buffer[80];

  strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", localTime);

  string fTime(buffer);

  return "[" + fTime + "] " + msg;
}

void appLog(string msg) { appendFile(LOG_FILE, logMsg(msg)); }

void crashLog(string msg) { appendFile(CRASH_LOG, logMsg(msg)); }

void signHandler(int sigN) {
  if (fanController && fanController->getWorker()) fanController->stopWorker();

  appLog("fanControl stopped");

  exit(EXIT_SUCCESS);
}
