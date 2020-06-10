#ifndef _UTILS_H_
#define _UTILS_H_

#include <any>
#include <string>

bool isTruthy(std::any expr);
bool isEqual(std::any arg0, std::any arg1);
std::string stringify(std::any r);

#endif
