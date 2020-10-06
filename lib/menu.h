/* 
 *  Menu class declaration
 *  
 *  File: menu.h
 *  Author: b4fThrive
 *  Copyright (c) 2020 2020 b4f.thrive@gmail.com
 *  
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *  
 */

#ifndef _MENU_ /* _MENU_ */
#define _MENU_

#include <iostream>
#include <vector>

using namespace std;

struct MenuOpt {
  string text;
  int    nextOpt;

  MenuOpt(string, int, int);
  ~MenuOpt();
};

class Menu {
  int             opt, menuSize;
  string          title, subtitle;
  vector<MenuOpt> menuOpts;

public:
  Menu(string = "", int = 0);
  ~Menu();

  void clear();

  void setTitle(string);
  void setSubtitle(string);
  void newOpt(string, int);

  void printTitle();
  void printSubtitle();
  void printOptions();
  void printMenu();

  int printMenuAndgetOpt(int);

  int getOpt(int);
  int getSelected() const;
};

#endif /* _CONFIG_MENU_ */
