/* 
 *  Fan control main header
 *  Globals declarations
 *  
 *  File: main.h
 *  Author: b4fThrive
 *  Copyright (c) 2020 b4f.thrive@gmail.com
 *  
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *  
 */

#ifndef _FAN_CONTROL_
#define _FAN_CONTROL_

#include <iostream>
#include <map>

#include "Sensors.h"

using namespace std;

/******************************************************************************
 * Aplication globals
 ******************************************************************************/

extern const string APP_USER;  // User running the app
extern const string HOME_PATH; // User home
extern const string APP_PATH;  // User app folder
extern const string CFG_FILE;  // User app config file
extern const string LOG_FILE;  // User app log file
extern const string CRASH_LOG; // User app crashlog file
extern const string VAR_DIR;   // Var directory
extern const string PID_FILE;  // Service PID
extern const string USR_FILE;  // User running service

void readConfig(FanController *&fanCtl); // Reads config file
void writeConfig(FanController *fanCtl); // Writes config file

/******************************************************************************
 * Aplication commands
 ******************************************************************************/

enum appCommands {
  eComm = -2, // Error on command
  noComm,     // No command
  start,      // Start fan control worker
  stop,       // Stop fan control worker
  restart,    // Restart fan control worker
  status,     // Show fan control status
  config,     // Config wizard mode
  help,       // Show fan control help
  version     // Show fan control app version
};

extern map<string, int> commands;

/******************************************************************************
 * Aplication commands handler
 ******************************************************************************/

void startApp();     // Starts fanControl service
void stopApp();      // Stops fanControl service
void restartApp();   // Restarts fanControl service
void configWizard(); // Starts the config wizard
void appStatus();    // Show service status
void showHelp();     // Show help commands
void showVers();     // Show fan control app version

/******************************************************************************
 * Aplication logs and signal handler
 ******************************************************************************/

string logMsg(string msg);
void   appLog(string msg);
void   crashLog(string msg);
void   signHandler(int sigN);

#endif // _FAN_CONTROL_
