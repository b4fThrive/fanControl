#ifndef UTILS_
#define UTILS_

#include <iostream>
#include <vector>

using namespace std;

// Utilities/Helpers
namespace utils {

string checkDir(string &);
string readFile(string, bool = false);
bool   writeFile(string, string);
bool   appendFile(string, string);

void closeSTDdescriptors();

/**
 * System shell command class.
 * Can exec a system command and stores the result in a Container<string>.
 *
 * @class utils::ShellCommand
 */
class ShellCommand {
private:
  string         command; // command executed
  string         error;   // getLine() error
  vector<string> lines;   // data command result
  int            pos;     // position line
  unsigned int   size;    // number of lines on dataBuffer

public:
  ShellCommand(string = "");
  ~ShellCommand();

  bool   getLine(string &);
  bool   getLine(string &, unsigned int);
  string firsLine() const;
  string lastLine() const;
  int    currLine() const;
  int    getSize() const;

  bool exec(string = "");
};

bool cinToInt(int &);

/******************************************************************************
 * Types alias
 ******************************************************************************/

typedef ShellCommand ShComm;
} // namespace utils

#endif /* UTILS_ */
