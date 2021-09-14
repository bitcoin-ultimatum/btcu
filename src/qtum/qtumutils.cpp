#include <qtum/qtumutils.h>
#include <libdevcore/CommonData.h>
#include <pubkey.h>
#include <util/convert.h>
#include <main.h>

using namespace dev;

bool qtumutils::btc_ecrecover(const dev::h256 &hash, const dev::u256 &v, const dev::h256 &r, const dev::h256 &s, dev::h256 &key)
{
    // Check input parameters
    if(v >= 256)
    {
        // Does not fit into 1 byte
        return false;
    }

    // Convert the data into format usable for btc
    CPubKey pubKey;
    std::vector<unsigned char> vchSig;
    vchSig.push_back((unsigned char)v);
    vchSig += r.asBytes();
    vchSig += s.asBytes();
    uint256 message = uint256(h256Touint(hash).ToStringReverseEndian());
    // Recover public key from compact signature (65 bytes)
    // The public key can be compressed (33 bytes) or uncompressed (65 bytes)
    // Pubkeyhash is RIPEMD160 hash of the public key, handled both types
    if(pubKey.RecoverCompact(message, vchSig))
    {
        // Get the pubkeyhash
        CKeyID id = pubKey.GetID();
        size_t padding = sizeof(key) - sizeof(id);
        memset(key.data(), 0, padding);
        memcpy(key.data() + padding, id.begin(), sizeof(id));
        return true;
    }

    return false;
}

h256 qtumutils::btc_strhash2sha256(bytesConstRef _input) noexcept
{
   std::string str_hex = dev::toHex(_input).substr(0,64);
   CHashWriter ss(SER_GETHASH, 0);
   ss << strMessageMagic;
   ss << str_hex;
   uint256 msgHash = uint256(ss.GetHash().ToStringReverseEndian());
   h256 hash = uintToh256(msgHash);
   return hash;
}
