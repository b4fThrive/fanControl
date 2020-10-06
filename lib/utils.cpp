/* 
 *  File description
 *  
 *  File: utils.cpp
 *  Author: b4fThrive
 *  Copyright (c) 2020 2020 b4f.thrive@gmail.com
 *  
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *  
 */

#include "utils.h"

#include <cstdio>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace utils {

/**
 * Checks if a dir path ends with / and correct it if not
 *
 * @param  {string} &dir : Directori path to check
 * @return {string}      : dir
 */
string checkDir(string &dir) {
  if (dir[dir.size() - 1] != '/') dir.push_back('/');
  return dir;
}

/**
 * Open file path on read mode and return first line
 *
 * @param  {string} path : File path
 * @return {string}      : Return first line fro file
 */
string readFile(string path, bool no_throw) {
  string   result = "";
  ifstream rFile(path);

  if (rFile.is_open()) {
    getline(rFile, result);
    rFile.close();
  } else if (!no_throw)
    throw runtime_error("Can't open file " + path);

  return result;
}

/**
 * Write a string in a file
 *
 * @param  {string} path    : File path
 * @param  {string} content : Content to write
 *
 * @return {bool}           : True if done
 */
bool writeFile(string path, string content) {
  ofstream wFile(path);

  if (!wFile.is_open()) return false;

  wFile << content;
  wFile.close();

  return true;
}

/**
 * Write in append mode a string in a file
 *
 * @param  {string} path    : File path
 * @param  {string} content : Content to append
 *
 * @return {bool}           : True if done
 */
bool appendFile(string path, string content) {
  ofstream wFile(path, ios_base::app);

  if (!wFile.is_open()) return false;

  wFile << content << endl;
  wFile.close();

  return true;
}

// Close standard descriptors
void closeSTDdescriptors() {
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

ShellCommand::ShellCommand(string command) : command(command) { exec(); }
ShellCommand::~ShellCommand() {}

/**
 * System shell command class getter
 * If not _pos is gived then move to next line automatically
 *
 * @class  utils::ShellCommand
 * @public ShellCommand::getLine
 *
 * @param  {string} &line : Where to store the line readed
 *
 * @return {bool}         : True if done false if not
 */
bool ShellCommand::getLine(string &line) {
  if (size == 0) return false;

  if (pos == size) {
    pos = 0;
    return false;
  }

  line = lines[pos++];
  return true;
}

/**
 * System shell command class getter
 * If not _pos is gived then move to next line automatically
 *
 * @class  utils::ShellCommand
 * @public ShellCommand::getLine
 *
 * @param  {string}       &line : Where to store the line readed
 * @param  {unsigned int} pos   : Line number to get
 *
 * @return {bool}               : True if done false if not
 */
bool ShellCommand::getLine(string &line, unsigned int _pos) {
  if (size == 0 || _pos >= size) return false;

  line = lines[_pos];
  return true;
}

string ShellCommand::firsLine() const { return size == 0 ? "" : lines[0]; }
string ShellCommand::lastLine() const {
  return size == 0 ? "" : lines[size - 1];
}

/**
 * System shell command class getter
 * Return current line number
 *
 * @class  utils::ShellCommand
 * @public currLine()
 *
 * @return {int} : Current line number or -1
 */
int ShellCommand::currLine() const { return pos; }

/**
 * System shell command class getter
 * Return total number of lines
 *
 * @class  utils::ShellCommand
 * @public getSize()
 *
 * @return {int} : Total number of lines
 */
int ShellCommand::getSize() const { return size; }

/**
 * System shell command class function
 * Exec _command and set it as new command or the command stored if empty
 *
 * @class  utils::ShellCommand
 * @public exec(string _command)
 *
 * @param  {string} _command : Command to execute
 *
 * @return {bool}            : True if done
 */
bool ShellCommand::exec(string _command) {
  lines.clear();
  size    = 0;
  command = _command != "" ? _command : command;
  error   = "";

  if (command == "") {
    error = "The commands is empty";
    return false;
  }

  string line;
  FILE * fsResult = popen(command.c_str(), "r");

  if (fsResult == NULL) {
    error = "Error processing command: " + command;
    return false;
  }

  char c = fgetc(fsResult);
  while ((c != -1)) {
    if (c != '\n') line.push_back(c);
    else {
      lines.push_back(line);
      line = "";
    }
    if ((c = fgetc(fsResult)) == -1 && line != "") lines.push_back(line);
  }

  fclose(fsResult);

  size = lines.size();

  if (size == 0) {
    pos   = -1;
    error = "Command without any result";
    return false;
  }

  pos = 0;
  return true;
}

/**
 * Returs true if give an integer from cin
 * and stores the value in var
 *
 * @param  {int} var : Variable to store the number
 *
 * @return {bool}    : True if captured a integer
 */
bool cinToInt(int &var) {
  string strCin; // String control

  cin >> strCin;

  try {
    int value = stoi(strCin);
    var       = value;
  } catch (const exception &e) { return false; }

  return true;
}

} // namespace utils
