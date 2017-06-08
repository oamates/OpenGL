#ifndef CMS_TYPES_H
#define CMS_TYPES_H

#include <vector>
#include <cstdint>

namespace cms
{

typedef unsigned int uint;
typedef unsigned long long int ullint;
typedef std::vector<uint> uintVec;
typedef std::vector<uintVec> uintVecVec;
typedef std::vector<int> intVec;
typedef std::vector<int8_t> int8Vec;
typedef std::vector<float> floatVec;

}

#endif //CMS_TYPES_H
