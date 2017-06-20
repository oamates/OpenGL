#ifndef _cms_range_included_879127560376561082035617354682375423754626723546827
#define _cms_range_included_879127560376561082035617354682375423754626723546827

#include <cassert>

namespace cms
{

struct Range
{
    int m_lower;
    int m_upper;

    Range() {}

    Range(int i_lower, int i_upper)
        : m_lower(i_lower), m_upper(i_upper)
    {
        assert(m_lower <= m_upper);
    }

};

} // namespace cms

#endif // _cms_range_included_879127560376561082035617354682375423754626723546827
