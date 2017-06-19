#ifndef _cms_address_5981234562873460175920501725396152345301725670325378123568
#define _cms_address_5981234562873460175920501725396152345301725670325378123568

#include <cstddef>
#include <array>

extern const int ADDRESS_SIZE;

namespace cms
{

//=======================================================================================================================================================================================================================
// structure that stores the address of a cell in a fixed-length byte array
//=======================================================================================================================================================================================================================

struct address_t
{
    std::vector<uint8_t> m_rawAddress;                                          // Storing the actual address
//    std::array<uint8_t, ADDRESS_SIZE> m_rawAddress;                                          // Storing the actual address

    address_t()
        { m_rawAddress.resize(ADDRESS_SIZE); }
    
    void set(const std::vector<uint8_t>& parentAddressPtr, uint8_t position_in_parent) // Set the address based on the parent's such and the cell's position in the parent
    {
        for(int32_t i = 0; i < ADDRESS_SIZE; ++i)
        {
            if(parentAddressPtr[i] != 0)                                        // Copy the parent's address
                m_rawAddress[i] = parentAddressPtr[i];
            else
            {
                m_rawAddress[i] = position_in_parent;                           // Add the new position in parent to the address
                break;                                                          // Avoid any further assignments
            }
        }
    }
    
    void reset()                                                                // Resets all the address to zero for the full size of the address
        { m_rawAddress.assign(ADDRESS_SIZE, 0); }
    
    //===================================================================================================================================================================================================================
    // computes unique hash value of the address
    // elements of m_rawAddress are < 8, so octal value m_rawAddress of can be used for ADDRESS_SIZE <= 10 
    //===================================================================================================================================================================================================================
    uint32_t hash()
    {
        uint32_t formattedAddress = 0;
        for(int32_t i = ADDRESS_SIZE - 1; i >= 0; --i)
            formattedAddress = (formattedAddress << 3) + m_rawAddress[i];
        return formattedAddress;        
    }

};

} // namespace cms

#endif // _cms_address_5981234562873460175920501725396152345301725670325378123568