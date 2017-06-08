#ifndef _cms_address_5981234562873460175920501725396152345301725670325378123568
#define _cms_address_5981234562873460175920501725396152345301725670325378123568

#include <cstddef>

#include "util.hpp"

extern int ADDRESS_SIZE;

namespace cms
{

// Stores the address of a cell in an array of consecutive uint8_t(s)

struct Address
{
    uint32_t m_maxAddressSize;                                                  // The max address size based on the max depth of the octree
    std::vector<uint8_t> m_rawAddress;                                          // Storing the actuall address

    Address()
        { m_rawAddress.resize(ADDRESS_SIZE); }
    
    void set(const std::vector<uint8_t>& parentAddressPtr, uint8_t posInParent) // Set the address based on the parent's such and the cell's position in the parent
    {
        for(int32_t i = 0; i < ADDRESS_SIZE; ++i)
        {
            if(parentAddressPtr[i] != 0)                                        // Copy the parent's address
            {
                m_rawAddress[i] = parentAddressPtr[i];
            }
            else
            {
                m_rawAddress[i] = posInParent;                                  // Add the new position in parent to the address
                break;                                                          // Avoid any further assignments
            }
        }
    }
    
    void reset()                                                                // Resets all the address to zero for the full size of the address
        { m_rawAddress.assign(ADDRESS_SIZE, 0); }
    
    void populateAddress(const std::vector<uint8_t>& rawAddress)                // Populate address with an existing raw address
        { m_rawAddress = rawAddress; }

    // Retrieves the address as a single uint
    // We use an unsigned integers (uint) which is 32-bit on all* platforms and can store a range of 4 billion
    // which is 10 digits. Thus can safely be used for up to depth 9 (2^9 == 512 samples).
    // Thus max address == 888888888
    uint32_t getFormatted()
        { return formatAddress(); }
    
    const std::vector<uint8_t>& getRaw()                                        // Return const ref to the raw address vector
        { return m_rawAddress; }
    
    uint32_t formatAddress()                                                    // Formats the raw address into a single long integer
    {
        uint32_t formattedAddress = 0;


        for(int32_t i = ADDRESS_SIZE - 1; i >= 0; --i)
        {
            if (m_rawAddress[i])
                formattedAddress += m_rawAddress[i] * (util::intPower(10, i));
        }
        return formattedAddress;
    }

};

} // namespace cms

#endif // _cms_address_5981234562873460175920501725396152345301725670325378123568