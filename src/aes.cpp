/*
 * This was created from the Crypto++ wiki, might be able to make improvements
 */

#include <cryptopp/hex.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <cryptopp/osrng.h>

#include <assert.h>
#include <string>

#include "aes.h"
#include "base64.h"
#include "logger.h"

static const int TAG_SIZE = 16;

AESPacket encrypt(const byte *k, const std::string data)
{
    byte key[32];
    memcpy(key, k, sizeof(key));

    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);

    CryptoPP::AutoSeededRandomPool rnd;
    rnd.GenerateBlock(iv, iv.size());

    std::string auth;

    // Hash the message
    CryptoPP::SHA256 hash;
    hash.Update((const byte *)data.data(), data.size());
    auth.resize(hash.DigestSize());
    hash.Final((byte *)&auth[0]);

    // Encrypted, with Tag
    std::string cipher, encoded;

    try
    {
        CryptoPP::GCM<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(key, sizeof(key), iv, iv.size());

        // AuthenticatedEncryptionFilter
        CryptoPP::AuthenticatedEncryptionFilter ef(e,
                                                   new CryptoPP::StringSink(cipher), false, TAG_SIZE);

        ef.ChannelPut("AAD", (const byte *)auth.data(), auth.size());
        ef.ChannelMessageEnd("AAD");

        ef.ChannelPut("", (const byte *)data.data(), data.size());
        ef.ChannelMessageEnd("");

        // Encode in hex
        // CryptoPP::StringSource(cipher, true,
        //              new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded), true, 16, " "));
    }
    catch (CryptoPP::BufferedTransformation::NoChannelSupport &e)
    {
        logWarning("Error encrypting message, Caught NoChannelSupport: " + std::string(e.what()));
    }
    catch (CryptoPP::AuthenticatedSymmetricCipher::BadState &e)
    {
        logWarning("Error encrypting message, Caught BadState: " + std::string(e.what()));
    }
    catch (CryptoPP::InvalidArgument &e)
    {
        logWarning("Error encrypting message, Caught InvalidArgument: " + std::string(e.what()));
    }

    AESPacket p = {cipher, auth, iv};

    return p;
}

std::string decrypt(const byte *k, const AESPacket p)
{
    byte key[32];
    memcpy(key, k, sizeof(key));

    CryptoPP::SecByteBlock iv(p.iv.data(), p.iv.size());

    std::string message, cipher = p.cipher;
    std::string auth = p.hash;

    try
    {
        CryptoPP::GCM<CryptoPP::AES>::Decryption d;
        d.SetKeyWithIV(key, sizeof(key), iv, iv.size());

        // Break the cipher text out into it's
        //  components: Encrypted Data and MAC Value
        std::string enc = cipher.substr(0, cipher.length() - TAG_SIZE);
        std::string mac = cipher.substr(cipher.length() - TAG_SIZE);

        // Sanity checks
        assert(cipher.size() == enc.size() + mac.size());
        assert(TAG_SIZE == mac.size());

        CryptoPP::AuthenticatedDecryptionFilter df(d, NULL,
                                                   CryptoPP::AuthenticatedDecryptionFilter::MAC_AT_BEGIN |
                                                       CryptoPP::AuthenticatedDecryptionFilter::THROW_EXCEPTION,
                                                   TAG_SIZE);

        // The order of the following calls are important
        df.ChannelPut("", (const byte *)mac.data(), mac.size());
        df.ChannelPut("AAD", (const byte *)auth.data(), auth.size());
        df.ChannelPut("", (const byte *)enc.data(), enc.size());

        // If the object throws, it will most likely occur during ChannelMessageEnd()
        df.ChannelMessageEnd("AAD");
        df.ChannelMessageEnd("");

        // If the object does not throw, here's the only opportunity to check the data's integrity
        bool b = df.GetLastResult();
        assert(b);

        // Remove data from channel
        size_t n = (size_t)-1;

        // Recover text from enc.data()
        df.SetRetrievalChannel("");
        n = (size_t)df.MaxRetrievable();
        message.resize(n);

        if (n > 0)
        {
            df.Get((byte *)message.data(), n);
        }
    }
    catch (CryptoPP::InvalidArgument &e)
    {
        logWarning("Error decrypting message, Caught InvalidArgument: " + std::string(e.what()));
    }
    catch (CryptoPP::AuthenticatedSymmetricCipher::BadState &e)
    {
        logWarning("Error decrypting message, Caught BadState: " + std::string(e.what()));
    }
    catch (CryptoPP::HashVerificationFilter::HashVerificationFailed &e)
    {
        logWarning("Error decrypting message, Caught HashVerificationFailed: " + std::string(e.what()));
    }

    return message;
}