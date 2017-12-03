// Class that allows to treat strings

#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

#include <string>
#include <vector>

struct CStringUtils
{
    /// Trim right.
    static std::string TrimRight(const std::string& Source, const std::string& T = " ");

    /// Trim left.
    static std::string TrimLeft(const std::string& Source, const std::string& T = " ");

    /// Trim left and right.
    static std::string Trim(const std::string& Source, const std::string& T = " ");

  private:
    CStringUtils(void);
    ~CStringUtils(void);
};

#endif
