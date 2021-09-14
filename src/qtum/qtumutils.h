#ifndef QTUMUTILS_H
#define QTUMUTILS_H

#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

/**
 * qtumutils Provides utility functions to EVM for functionalities that already exist in qtum
 */
namespace qtumutils
{
/**
 * @brief btc_ecrecover Wrapper to CPubKey::RecoverCompact
 */
bool btc_ecrecover(dev::h256 const& hash, dev::u256 const& v, dev::h256 const& r, dev::h256 const& s, dev::h256 & key);

/**
 * @brief btc_strhash2sha256 btc CHashWriter wrapper
 */
dev::h256 btc_strhash2sha256(dev::bytesConstRef _input) noexcept;
}

#endif
