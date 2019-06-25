#ifndef AES_H
#define AES_H

struct AESPacket
{
    std::string cipher;
    std::string hash;
    CryptoPP::SecByteBlock iv;
};

AESPacket encrypt(const byte *k, const std::string data);
std::string decrypt(const byte *k, const AESPacket p);

#endif