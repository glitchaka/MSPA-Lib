#pragma once
#include <cstdlib>
#include <iostream>
template <typename T> inline void require(T&& value,const char* msg){if(!static_cast<bool>(value)){std::cerr<<"FAIL: "<<msg<<'\n';std::exit(1);}}
