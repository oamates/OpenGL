#include <iostream>

#include "util.hpp"
#include "address.hpp"


extern int ADDRESS_SIZE;

namespace cms
{


Address::Address()
{
  m_rawAddress.resize(ADDRESS_SIZE);
}

//---------------------------------------------------------------

void Address::populateAddress(const std::vector<uint8_t>& rawAddress)
{
  m_rawAddress = rawAddress;
}


//---------------------------------------------------------------

void Address::reset()
{
  m_rawAddress.assign(ADDRESS_SIZE, 0);
}

//---------------------------------------------------------------

void Address::set(const std::vector<uint8_t>& parentAddressPtr, uint8_t posInParent)
{
  for(int i=0; i<ADDRESS_SIZE; ++i)
  {
    // Copy the parent's address
    if(parentAddressPtr[i] != 0)
    {
      m_rawAddress[i] = parentAddressPtr[i];
    }
    else
    {
      // Add the new position in parent to the address
      m_rawAddress[i] = posInParent;

      // Avoid any further assignments
      break;
    }
  }
}

//---------------------------------------------------------------

uint Address::getFormatted()
{
  return formatAddress();
}

//---------------------------------------------------------------

const std::vector<uint8_t>& Address::getRaw()
{
  return m_rawAddress;
}

//---------------------------------------------------------------

uint Address::formatAddress()
{
  uint formattedAddress = 0;


  for(int i=ADDRESS_SIZE-1; i>=0; --i)
  {
    if(m_rawAddress[i])
      formattedAddress += m_rawAddress[i] * (util::intPower(10, i));
  }


  return formattedAddress;
}


} // namespace cms
