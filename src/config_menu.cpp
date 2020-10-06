/* 
 *  File description
 *  
 *  File: config_menu.cpp
 *  Author: b4fThrive
 *  Copyright (c) 2020 2020 b4f.thrive@gmail.com
 *  
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *  
 */

#include "config_menu.h"
#include "Sensors.h"
#include "menu.h"
#include "shell_commands.h"
#include "utils.h"

#include <iostream>
#include <vector>

using namespace std;
using namespace utils;

ConfigMode::ConfigMode(string menuTitle)
    : SYS_DEVS(new SystemDevices()), fanCtlWiz(nullptr), fanCtlCfg(nullptr),
      menu(Menu(menuTitle)), stage(start_menu), is_new_fan(false),
      selectedSensor(nullptr), selectedFan(nullptr) {
  for (int i = 0; i < SYS_DEVS->nHwmonDevs; i++) {
    HwmonDevice *dev         = SYS_DEVS->hwmonDevices[i];
    hwmSens_vp * sensors     = &dev->sensors;
    fans_vp *    fans        = &dev->fans;
    int          sensorsSize = sensors->size();
    int          fansSize    = fans->size();
    int          maxSize     = max(fansSize, sensorsSize);

    for (int j = 0; j < maxSize; j++) {
      if (j < sensorsSize) {
        HwMonSensor *sensor = (*sensors)[j];
        HWMON_S.push_back(sensor);
        SENSORS.push_back(sensor);
      }
      if (j < fansSize) FANS.push_back((*fans)[j]);
    }
  }

  for (int i = 0; i < SYS_DEVS->nDisks; i++) {
    HddTempSensor *sensor = SYS_DEVS->hddtempSensors[i];
    HDDTEMP_S.push_back(sensor);
    SENSORS.push_back(sensor);
  }

  sysFansSz = FANS.size();
  sysSensSz = SENSORS.size();
}

ConfigMode::~ConfigMode() {
  int hwmonSize = HWMON_S.size();

  if (fanCtlCfg && fanCtlWiz) {
    if (fanCtlCfg->getAmbSensor() == fanCtlWiz->getAmbSensor())
      fanCtlWiz->setAmbSensor(nullptr, false);
    if (fanCtlCfg->getFans() == fanCtlWiz->getFans())
      fanCtlWiz->setFans(nullptr, false);
  }

  if (fanCtlCfg) {
    fanCtlCfg->clearAll();
    delete fanCtlCfg;
    fanCtlCfg = nullptr;
  }

  if (fanCtlWiz) {
    fanCtlWiz->clearAll();
    delete fanCtlWiz;
    fanCtlWiz = nullptr;
  }

  if (selectedFan && is_new_fan) delete selectedFan;

  selectedFan    = nullptr;
  selectedSensor = nullptr;

  for (int i = 0; i < hwmonSize; i++) HWMON_S[i] = nullptr;
  for (int i = 0; i < SYS_DEVS->nDisks; i++) HDDTEMP_S[i] = nullptr;
  for (int i = 0; i < SYS_DEVS->nFans; i++) FANS[i] = nullptr;
  for (int i = 0; i < sysSensSz; i++) SENSORS[i] = nullptr;

  HWMON_S.clear();
  HDDTEMP_S.clear();
  FANS.clear();
  SENSORS.clear();

  delete SYS_DEVS;
}

int ConfigMode::startMnu() {
  ShComm shell;
  bool   foundConfig = shell.exec(LS(CFG_FILE));

  menu.newOpt("Create a new config", new_config_menu);
  if (foundConfig) {
    readConfig(fanCtlCfg);
    menu.newOpt("Edit current config", edit_config_menu);
    menu.newOpt("Delete current config", del_config_menu);
    menu.newOpt("Show current config", show_config_menu);
  }
  exitOpt();

  return stage = menu.printMenuAndgetOpt(start_menu);
}

int ConfigMode::configMnu() {
  bool has_amb_sensor = fanCtlWiz && fanCtlWiz->getAmbSensor();
  int  configuredFans = !fanCtlWiz ? 0 : fanCtlWiz->getFans()->size();
  bool oneConfigured  = configuredFans > 0 &&
                       fanCtlWiz->getFans()->at(0)->getSensors()->size() > 0;

  menu.setSubtitle(stage == new_config_menu
                       ? "WARNING: SAVE A NEW CONFIG WILL OVEWRITE ANY CONFIG "
                         "YOU CURRENTLY HAVE!!!\n\nCreating a config."
                       : "Editing current config.");

  buildStartOpts();

  if (has_amb_sensor) {
    menu.newOpt("Change the ambient sensor.", set_amb_sens_menu);
    menu.newOpt("Remove the ambient sensor.", confirm_del_amb_sensor);
  } else
    menu.newOpt("Add an ambient sensor.", set_amb_sens_menu);

  if (configuredFans < sysFansSz) menu.newOpt("Add a new fan.", add_fans_menu);
  if (configuredFans > 0) {
    menu.newOpt("Edit configured fan.", edit_fans_menu);
    menu.newOpt("Delete configured fan.", del_fans_menu);
  }

  if (cfgMode == modify || oneConfigured)
    menu.newOpt("Save config", confirm_save_config);

  exitOpt();

  return stage = menu.printMenuAndgetOpt(stage);
}

int ConfigMode::selectFanMnu() {
  int nextStage, stgOpt;

  if (stage == del_fans_menu) {
    menu.setSubtitle("Delete fan menu");
    nextStage = confirm_del_fan;
  } else {
    menu.setSubtitle(stage == add_fans_menu ? "Add fan menu" : "Edit fan menu");
    nextStage = stage == add_fans_menu ? add_sensors_menu : config_fan_menu;
  }

  buildStartOpts();
  buildFanOpts();
  exitOpt();

  if ((stgOpt = menu.printMenuAndgetOpt(stage)) >= indexedOpt) {
    if (stage == add_fans_menu)
      selectedFan = new FanNode(FANS[stgOpt - indexedOpt]);
    else
      selectedFan = (*fanCtlWiz->getFans())[stgOpt - indexedOpt];
    stage = nextStage;
  } else
    stage = stgOpt;

  return stage;
}

int ConfigMode::configFanMnu() {
  int  configuredSens = selectedFan->getSensors()->size();
  bool oneConfigured  = configuredSens > 0;

  menu.setSubtitle("Configuring fan: " + labels_str(selectedFan->getFan()));

  buildStartOpts();
  if (configuredSens < sysSensSz)
    menu.newOpt("Add a new sensor", add_sensors_menu);
  if (configuredSens > 0) {
    menu.newOpt("Edit a sensor", edit_sensors_menu);
    menu.newOpt("Delete a sensor", del_sensors_menu);
  }
  if (oneConfigured)
    menu.newOpt("End fan configuration",
                is_new_fan
                    ? confirm_add_fan
                    : cfgMode == create ? new_config_menu : edit_config_menu);
  exitOpt();

  return stage = menu.printMenuAndgetOpt(stage);
}

int ConfigMode::selectSensorMnu() {
  int nextStage, stgOpt;

  switch (stage) {
    case set_amb_sens_menu:
      menu.setSubtitle("Selecting an ambient sensor.");
      nextStage = confirm_set_amb_sensor;
      break;

    case add_sensors_menu:
      menu.setSubtitle("Selecting sensor to associate with " +
                       labels_str(selectedFan->getFan()));
      nextStage = set_values_add_sensor;
      break;

    case edit_sensors_menu:
      menu.setSubtitle("Selecting associated sensor to edit on " +
                       labels_str(selectedFan->getFan()));
      nextStage = set_values_edit_sensor;
      break;

    case del_sensors_menu:
      nextStage = confirm_del_sensor;
      menu.setSubtitle("Selecting associated sensor to remove on " +
                       labels_str(selectedFan->getFan()));
      break;
  }

  buildStartOpts();
  buildSensorOpts();
  exitOpt();

  if ((stgOpt = menu.printMenuAndgetOpt(stage)) >= indexedOpt) {
    selectedSensor = stage == edit_sensors_menu || stage == del_sensors_menu
                         ? (*selectedFan->getSensors())[stgOpt - indexedOpt]
                         : SENSORS[stgOpt - indexedOpt];
    stage = nextStage;
  } else
    stage = stgOpt;

  return stage;
}

int ConfigMode::setTempMnu(int type) {
  string typeStr = type == minTemp
                       ? "minimum working"
                       : type == maxTemp ? "maximum working" : "ambient offset";
  int temp = 0;

  while (temp < 1 || temp > 100) {
    menu.printTitle();
    cout << "Configuring " << labels_str(selectedSensor) << ", on "
         << labels_str(selectedFan->getFan()) << "." << endl
         << endl;
    cout << "Type " + typeStr + " temperature: ";

    if (!cinToInt(temp)) temp = 0;
  }

  if (type == minTemp) {
    selectedSensor->setMinTemp(temp);
    return setTempMnu(maxTemp);
  }

  if (type == maxTemp) {
    selectedSensor->setMaxTemp(temp);
    return setTempMnu(offsetTemp);
  }

  selectedSensor->setoffsetT(temp);
  return stage = stage == set_values_add_sensor ? confirm_add_sensor
                                                : edit_sensors_menu;
}

bool ConfigMode::confirmSaveConfig() {
  string ask = "";

  while (ask != "yes" && ask != "no") {
    menu.printTitle();
    printFanControllerWizard(false);
    cout << endl << "Do you wanna save the configuration? (yes/no)" << endl;
    cin >> ask;
  }

  return ask == "yes";
}

bool ConfigMode::confirm(string msg) {
  string ask;

  while (ask != "yes" && ask != "no") {
    menu.printTitle();
    cout << msg << " (yes/no)" << endl;
    cin >> ask;
  }

  return ask == "yes";
}

void ConfigMode::printFanControllerWizard(bool pause) {
  fanNode_vp *fans      = fanCtlWiz->getFans();
  Sensor *    ambSensor = fanCtlWiz->getAmbSensor();
  int         fansSize  = fans->size();

  cout << "Ambient sensor: " << labels_str(ambSensor) << "." << endl;

  for (int i = 0; i < fansSize; i++) {
    sensors_vp *sensors     = (*fans)[i]->getSensors();
    int         sensorsSize = sensors->size();

    cout << labels_str((*fans)[i]->getFan()) << "." << endl;

    if (sensorsSize > 0) cout << "                Sensors:" << endl;

    for (int j = 0; j < sensorsSize; j++) {
      Sensor *sensor = (*sensors)[j];

      cout << "                 " << labels_str(sensor)
           << ". Min temp: " << to_string(sensor->getMinT() / 1000)
           << " | Max temp: " << to_string(sensor->getMaxT() / 1000)
           << " | Offset temp: " << to_string(sensor->getOffsetT() / 1000)
           << endl;
    }
  }

  if (pause) cin.ignore().get();
}

void ConfigMode::startOpt() { menu.newOpt("Return start menu.", start_menu); }

void ConfigMode::returnBackOpt() {
  switch (stage) {
    case add_fans_menu:
    case edit_fans_menu:
    case del_fans_menu:
    case set_amb_sens_menu:
      if (cfgMode == create)
        menu.newOpt("Return back new config menu", new_config_menu);
      else
        menu.newOpt("Return back edit config menu", edit_config_menu);
      break;

    case config_fan_menu:
      if (is_new_fan) menu.newOpt("Return back add fans menu", add_fans_menu);
      else
        menu.newOpt("Return back edit fan menu", edit_fans_menu);
      break;

    case add_sensors_menu:
    case edit_sensors_menu:
    case del_sensors_menu:
      menu.newOpt("Return back config fan", config_fan_menu);
      break;
  }
}

void ConfigMode::exitOpt() { menu.newOpt("Exit config mode", exit_menu); }

void ConfigMode::buildStartOpts() {
  startOpt();
  returnBackOpt();
}

void ConfigMode::buildEndOpts() {
  returnBackOpt();
  exitOpt();
}

void ConfigMode::buildFanOpts() {
  fanNode_vp *fans     = fanCtlWiz->getFans();
  int         fanCtlSz = fans->size();
  int         fansSize = FANS.size();

  if (stage == add_fans_menu) {
    for (int i = 0; i < fansSize; i++) {
      Fan *fan        = FANS[i];
      bool configured = false;
      int  fanCtlIndd = -1;

      while (!configured && ++fanCtlIndd < fanCtlSz)
        configured =
            (*fans)[fanCtlIndd]->getFan()->getDevName() == fan->getDevName() &&
            (*fans)[fanCtlIndd]->getFan()->getName() == fan->getName();

      if (!configured)
        menu.newOpt("Add " + labels_str(fan) + ".", indexedOpt + i);
    }
  } else {
    string pre = stage == edit_fans_menu ? "Edit " : "Remove ";

    for (int i = 0; i < fanCtlSz; i++)
      menu.newOpt(pre + labels_str((*fans)[i]->getFan()) + ".", indexedOpt + i);
  }
}

void ConfigMode::buildSensorOpts() {
  string      prefix, sufix;
  Fan *       fan     = selectedFan ? selectedFan->getFan() : nullptr;
  sensors_vp *sensors = !selectedFan ? nullptr : selectedFan->getSensors();
  int         sensSz  = !sensors ? 0 : sensors->size();
  int         type    = Sensor::abstract;

  if (stage == edit_sensors_menu || stage == del_sensors_menu) {
    if (stage == edit_sensors_menu) {
      prefix = "Modify ";
      sufix  = " on ";
    } else {
      prefix = "Remove ";
      sufix  = " on ";
    }

    sufix += labels_str(fan);

    for (int i = 0; i < sensSz; i++)
      menu.newOpt(prefix + labels_str((*sensors)[i]) + sufix, indexedOpt + i);
  } else {
    bool addToFan = stage == add_sensors_menu;

    if (stage == add_sensors_menu) {
      prefix = "Associate ";
      sufix  = " with " + labels_str(fan);
    } else {
      prefix = "Select ";
      sufix  = " as your ambient sensor.";
      type   = Sensor::hwmon;
    }

    for (int i = 0; i < sysSensSz; i++) {
      Sensor *sensor = SENSORS[i];

      if (type == Sensor::abstract || type == sensor->type) {
        bool selected = false;
        int  sensInd  = -1;

        while (addToFan && !selected && ++sensInd < sensSz)
          selected =
              sensor->getDevName() == (*sensors)[sensInd]->getDevName() &&
              sensor->getName() == (*sensors)[sensInd]->getName();

        if (!selected)
          menu.newOpt(prefix + labels_str(sensor) + sufix, indexedOpt + i);
      }
    }
  }
}

string ConfigMode::labels_str(Sensor *sens) {
  string label  = sens->getLabel();
  string cLabel = sens->getCLabel();
  string result = cLabel.compare(label) >= 0 ? cLabel
                                             : label == cLabel || cLabel == ""
                                                   ? label
                                                   : label + "(" + cLabel + ")";
  return result + " sensor, from " + sens->getDevName() + " device";
}

string ConfigMode::labels_str(Fan *fan) {
  string label  = fan->getLabel();
  string cLabel = fan->getCLabel();
  string result = cLabel.compare(label) >= 0 ? cLabel
                                             : label == cLabel || cLabel == ""
                                                   ? label
                                                   : label + "(" + cLabel + ")";
  return result + " fan, from " + fan->getDevName() + " device";
}

void ConfigMode::start() {
  while (stage != exit_menu) {
    menu.clear();

    switch (stage) {
      case start_menu:
        if (fanCtlWiz) delete fanCtlWiz;
        if (fanCtlCfg) delete fanCtlCfg;
        fanCtlWiz = nullptr;
        fanCtlCfg = nullptr;
      case new_config_menu:
      case edit_config_menu:
        if (selectedFan && is_new_fan) delete selectedFan;
        selectedFan    = nullptr;
        selectedSensor = nullptr;
        is_new_fan     = false;

        if (stage == start_menu) {
          startMnu();
          break;
        }

        cfgMode = stage == new_config_menu ? create : modify;
        if (!fanCtlWiz)
          fanCtlWiz = cfgMode == create ? new FanController
                                        : new FanController(fanCtlCfg);

        configMnu();
        break;

      case set_amb_sens_menu:
      case add_sensors_menu:
      case edit_sensors_menu:
      case del_sensors_menu:
        selectedSensor = nullptr;
        selectSensorMnu();
        break;

      case confirm_del_amb_sensor: selectedSensor = nullptr;
      case confirm_set_amb_sensor:
        if (confirm(!selectedSensor ? "Do you wanna remove "
                                      "your current ambient sensor?"
                                    : "Do you wanna add the selected "
                                      "sensor as your ambient sensor?"))
          fanCtlWiz->setAmbSensor(selectedSensor);
        stage = cfgMode == create ? new_config_menu : edit_config_menu;
        break;

      case add_fans_menu:
      case edit_fans_menu:
      case del_fans_menu:
        if (selectedFan && is_new_fan) delete selectedFan;
        selectedFan = nullptr;
        is_new_fan  = stage == add_fans_menu;

        selectFanMnu();
        break;

      case config_fan_menu: configFanMnu(); break;

      case confirm_save_config:
        if (confirmSaveConfig()) {
          writeConfig(fanCtlWiz);
          stage = start_menu;
        } else
          stage = cfgMode == create ? new_config_menu : edit_config_menu;
        break;

      case confirm_add_fan:
        if (confirm("Do you want to add " +
                    labels_str(selectedFan->getFan()))) {
          fanCtlWiz->pushBackFanNode(selectedFan);
          is_new_fan = false;
        }

        stage = fanCtlWiz->getFans()->size() < sysFansSz
                    ? add_fans_menu
                    : cfgMode == create ? new_config_menu : edit_config_menu;
        break;

      case confirm_del_fan:
        if (confirm("Do you want to delete " +
                    labels_str(selectedFan->getFan()))) {
          int  fansSize = fanCtlWiz->getFans()->size();
          int  i        = -1;
          bool notFound = true;

          while (i < fansSize && selectedFan != (*fanCtlWiz->getFans())[i]) ++i;

          (*fanCtlWiz->getFans())[i] = (*fanCtlWiz->getFans())[fansSize - 1];
          fanCtlWiz->getFans()->pop_back();
          delete selectedFan;
          selectedFan = nullptr;
        }
        stage = fanCtlWiz->getFans()->size() > 0
                    ? del_fans_menu
                    : cfgMode == create ? new_config_menu : edit_config_menu;
        break;

      case set_values_add_sensor:
      case set_values_edit_sensor: setTempMnu(); break;

      case confirm_add_sensor:
        if (confirm("Do you wanna associate " + labels_str(selectedSensor) +
                    " to " + labels_str(selectedFan->getFan()))) {
          selectedFan->pushBackSensor(selectedSensor);
        }
        stage = add_sensors_menu;
        break;

      case confirm_del_sensor:
        if (confirm("Do you want to delete " + labels_str(selectedSensor) +
                    " on " + labels_str(selectedFan->getFan()))) {
          sensors_vp *sensors     = selectedFan->getSensors();
          int         sensorsSize = sensors->size();
          int         i           = -1;
          while (++i < sensorsSize)
            if ((*sensors)[i] == selectedSensor) {
              (*sensors)[i] = (*sensors)[sensorsSize - 1];
              sensors->pop_back();
              i = sensorsSize;
            }
        }
        stage = del_sensors_menu;
        break;

      case del_config_menu:
        if (confirm("Are you sure you want to delete the current config?")) {
          ShComm shell;
          if (shell.exec(RM(CFG_FILE))) cout << "Config deleted succesfully";
          else
            cout << "Error: Can't delete current config";

          cout << "Type Enter to continue";
          cin.ignore().get();
        }
        stage = start_menu;
        break;

      case show_config_menu:
        fanCtlWiz = new FanController(fanCtlCfg);
        menu.printTitle();
        printFanControllerWizard();
        stage = start_menu;
        break;

      default: stage = exit_menu;
    }
  }
}
