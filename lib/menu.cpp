/* 
 *  File description
 *  
 *  File: menu.cpp
 *  Author: b4fThrive
 *  Copyright (c) 2020 2020 b4f.thrive@gmail.com
 *  
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *  
 */

#include "menu.h"
#include "utils.h"

#include <iostream>
#include <vector>

using namespace std;
using namespace utils;

/******************************************************************************
 * Class MenuOpt
 ******************************************************************************/

MenuOpt::MenuOpt(string text, int opt, int nextOpt)
    : text((opt < 10 ? "   " : "  ") + to_string(opt) + ") " + text),
      nextOpt(nextOpt) {}
MenuOpt::~MenuOpt() {}

/******************************************************************************
 * Class Menu
 ******************************************************************************/

Menu::Menu(string title, int initOpt) : title(title), opt(initOpt) { clear(); }
Menu::~Menu() {}

void Menu::clear() {
  menuOpts.clear();
  opt      = 0;
  menuSize = 0;
  subtitle = "";
}

void Menu::setTitle(string _title) { title = _title; }
void Menu::setSubtitle(string _subtitle) { subtitle = _subtitle; }
void Menu::newOpt(string text, int nextOpt) {
  MenuOpt newOption(text, ++menuSize, nextOpt);
  menuOpts.push_back(newOption);
}

void Menu::printTitle() {
  system("clear");
  cout << title << endl << endl;
}
void Menu::printSubtitle() {
  if (subtitle != "") cout << subtitle << endl << endl;
}
void Menu::printOptions() {
  for (int i = 0; i < menuSize; i++) cout << menuOpts[i].text << endl;
  cout << endl;
  cout << "Select an option: ";
}
void Menu::printMenu() {
  printTitle();
  printSubtitle();
  printOptions();
}

int Menu::printMenuAndgetOpt(int badOpt) {
  printMenu();
  return getOpt(badOpt);
}

int Menu::getOpt(int badOpt) {
  return opt = !cinToInt(opt) || opt < 1 || opt > menuSize
                   ? badOpt
                   : menuOpts[opt - 1].nextOpt;
}
int Menu::getSelected() const { return opt; }
