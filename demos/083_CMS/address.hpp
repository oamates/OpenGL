#ifndef CMS_ADDRESS_H
#define CMS_ADDRESS_H

#include <cstddef>
#include "types.hpp"

namespace cms
{


/// @class Address
/// @brief Stores the address of a cell in an array
/// of consecutive uint8_t(s)
///
/// Adopts the 'Big-Endian' notation
class Address
{
public:

  /// @brief Empty Ctor
  Address();

  /// @brief Set the address based on the parent's such
  /// and the cell's position in the parent
  void set(const std::vector<uint8_t>& parentAddressPtr, uint8_t posInParent);

  /// @brief Resets all the address to zero for the full size of the address
  void reset();

  /// @brief Populate address with an existing raw address
  void populateAddress(const std::vector<uint8_t>& rawAddress);

  /// @brief Retrieves the address as a single uint
  /// We use an unsigned integers (uint) which is 32-bit on
  /// all* platforms and can store a range of 4 billion
  /// which is 10 digits. Thus can safely be used for up to
  /// depth 9 (2^9 == 512 samples). Thus max address == 888888888
  uint getFormatted();

  /// @brief Return const ref to the raw address vector
  const std::vector<uint8_t>& getRaw();

private:

  /// @brief Formats the raw address into a single long integer
  uint formatAddress();

  /// @brief Storing the actuall address
  std::vector<uint8_t> m_rawAddress;

  /// @brief The max address size based on the max depth of the octree
  int m_maxAddressSize;
};



} // namespace cms

#endif // CMS_ADDRESS_H
