#ifndef _CONFIG_MENU_ /* _CONFIG_MENU_ */
#define _CONFIG_MENU_

#include "Sensors.h"
#include "main.h"
#include "menu.h"

#include <iostream>
#include <vector>

using namespace std;
using namespace utils;

class ConfigMode {
  SystemDevices *const SYS_DEVS;  // Devices
  hwmSens_vp           HWMON_S;   // Helper pointers to system devices
  hddtSens_vp          HDDTEMP_S; // Helper pointers to system devices
  fans_vp              FANS;      // Helper pointers to system devices
  sensors_vp           SENSORS;   // Helper pointers to system devices

  int sysFansSz; // System fans size
  int sysSensSz; // System sensors size

  enum cfgModes { create, modify };              // Create or modify config
  enum setTemp { minTemp, maxTemp, offsetTemp }; // set temp sensor options
  enum stages {                                  // menu stage control
    exit_menu = -1,                              // exit config mode

    start_menu,
    new_config_menu,
    edit_config_menu,
    del_config_menu,
    show_config_menu,

    set_amb_sens_menu,
    add_fans_menu,
    edit_fans_menu,
    del_fans_menu,

    config_fan_menu,

    add_sensors_menu,
    edit_sensors_menu,
    del_sensors_menu,

    set_values_add_sensor,
    set_values_edit_sensor,

    confirm_set_amb_sensor,
    confirm_del_amb_sensor,

    confirm_add_fan,
    confirm_del_fan,

    confirm_add_sensor,
    confirm_del_sensor,

    confirm_save_config,

    indexedOpt = 999, // indexed option control
  };

  FanController *fanCtlWiz; // Config wizard
  FanController *fanCtlCfg; // Saved config

  Menu menu; // Menu constructor

  int      cfgMode;        // Config mode control {create or edit}
  int      stage;          // Current stage control
  bool     is_new_fan;     // New fan node control
  FanNode *selectedFan;    // Selected fan control
  Sensor * selectedSensor; // Selected sensor control

  int startMnu();
  int configMnu();
  int selectFanMnu();
  int configFanMnu();
  int selectSensorMnu();
  int setTempMnu(int type = minTemp);

  bool confirmSaveConfig();
  bool confirm(string msg);

  void printFanControllerWizard(bool pause = true);

  void startOpt();
  void returnBackOpt();
  void exitOpt();

  void buildStartOpts();
  void buildEndOpts();
  void buildFanOpts();
  void buildSensorOpts();

  string labels_str(Sensor *);
  string labels_str(Fan *);

public:
  ConfigMode(string menuTitle = "");
  ~ConfigMode();

  void start();
};

#endif /* _CONFIG_MENU_ */
