#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/secblock.h>
#include <cryptopp/oids.h>
#include <cryptopp/asn.h>

#include <string>
#include <cstring>
#include <vector>
#include <thread>

#include "client.h"
#include "aes.h"
#include "config.h"
#include "logger.h"

void handleRequest(int sock, int attempt = -1);
void handShakeRod(std::string host);

struct PeerInfo
{
    int infractions = 0;
    int version = -1;
    CryptoPP::SecByteBlock key;
};

std::map<std::string, PeerInfo> peers;

Client::Client()
{
    CryptoPP::OID CURVE = CryptoPP::ASN1::secp256k1();
    CryptoPP::AutoSeededX917RNG<CryptoPP::AES> rng;

    CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CURVE);
    CryptoPP::SecByteBlock priv(dh.PrivateKeyLength()), pub(dh.PublicKeyLength());
    dh.GenerateKeyPair(rng, priv, pub);

    config.setPrivkey(priv);
    config.setPubkey(pub);
}

void Client::start()
{
    int sockLnd;
    struct sockaddr_in servAddrLnd, cliAddr;
    socklen_t cli;

    srand(time(NULL));

    sockLnd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockLnd < 0)
    {
        logFatal("Error creating socket");
        exit(1);
    }

    memset((char *)&servAddrLnd, '0', sizeof(servAddrLnd));

    servAddrLnd.sin_family = AF_INET;
    servAddrLnd.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servAddrLnd.sin_port = htons(config.getPort());

    int opt = 1;
    if (setsockopt(sockLnd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
    {
        logFatal("Error, setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }

    if (bind(sockLnd, (struct sockaddr *)&servAddrLnd, sizeof(servAddrLnd)) < 0)
    {
        logFatal("Error binding socket for lightning instance");
        exit(1);
    }

    listen(sockLnd, SOMAXCONN);
    cli = sizeof(cliAddr);
    int hostVersion = -1;

    int i;
    for (i = 0; i < config.getHosts().size(); ++i)
    {
        handShakeRod(config.getHosts().at(i));
    }

    this->running = true;
    this->stopped = false;

    while (this->running)
    {
        // Receive message from lnd
        int newSock = accept(sockLnd, (struct sockaddr *)&cliAddr, &cli);

        std::thread handle(handleRequest, newSock, -1);
        handle.detach();
    }

    close(sockLnd);

    logInfo("Shutting down");

    this->stopped = true;
}

void handShakeRod(std::string host)
{
    int pos = host.find(":");
    std::string ip = host.substr(0, pos);
    int port = std::stoi(host.substr(pos + 1));

    int sockRod;
    struct sockaddr_in servAddrRod;
    struct hostent *server;

    sockRod = socket(AF_INET, SOCK_STREAM, 0);

    if (sockRod < 0)
    {
        logFatal("Error creating socket");
        exit(1);
    }

    server = gethostbyname(ip.c_str());

    if (server == NULL)
    {
        logFatal("Host does not exist or is not accepting connections");
        exit(1);
    }

    memset((char *)&servAddrRod, '0', sizeof(servAddrRod));

    servAddrRod.sin_family = AF_INET;
    servAddrRod.sin_port = htons(port);

    bcopy((char *)server->h_addr, (char *)&servAddrRod.sin_addr.s_addr, server->h_length);

    int c = connect(sockRod, (struct sockaddr *)&servAddrRod, sizeof(servAddrRod));

    if (c < 0)
    {
        logFatal("Error connecting to host");
        exit(1);
    }

    unsigned short version = htons(0);
    int keySize = htonl(-config.getPubKey().size()); // Set keySize to negative to signify key handshake

    send(sockRod, &keySize, sizeof(keySize), 0);
    send(sockRod, &version, sizeof(version), 0);
    send(sockRod, config.getPubKey().data(), config.getPubKey().size(), 0);

    read(sockRod, &keySize, sizeof(keySize));
    read(sockRod, &version, sizeof(version));

    keySize = ntohl(keySize);
    version = ntohs(version);

    byte hostPub[keySize];
    read(sockRod, hostPub, keySize);

    CryptoPP::SecByteBlock sp(hostPub, keySize);

    CryptoPP::OID CURVE = CryptoPP::ASN1::secp256k1();
    CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CURVE);

    CryptoPP::SecByteBlock shared(dh.AgreedValueLength());
    dh.Agree(shared, config.getPrivKey(), sp);

    PeerInfo pi = {0, version, shared};
    peers.insert(std::pair<std::string, PeerInfo>(host, pi));

    close(sockRod);
}

void handleRequest(int sock, int attempt)
{
    char buffer[1024] = {0};
    size_t r = read(sock, buffer, sizeof(buffer));

    int i = attempt;
    while (i == attempt)
        i = rand() % config.getHosts().size(); // Don't need a securely random number for this

    std::string host = config.getHosts().at(i);

    CryptoPP::SecByteBlock shared = peers.at(host).key;

    int pos = host.find(":");
    std::string ip = host.substr(0, pos);
    int port = std::stoi(host.substr(pos + 1));

    // Connect to host
    int sockRod = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server = gethostbyname(ip.c_str());
    struct sockaddr_in servAddrRod;
    memset((char *)&servAddrRod, '0', sizeof(servAddrRod));
    servAddrRod.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&servAddrRod.sin_addr.s_addr, server->h_length);
    servAddrRod.sin_port = htons(port);
    connect(sockRod, (struct sockaddr *)&servAddrRod, sizeof(servAddrRod));

    // Encrypt message
    std::string str(buffer, r);
    AESPacket p = encrypt(shared.data(), str);

    // Send message
    std::string hashAndCipher = p.hash + p.cipher;
    int payloadLength = htonl(hashAndCipher.length());
    send(sockRod, &payloadLength, sizeof(payloadLength), 0);
    send(sockRod, p.iv.data(), p.iv.size(), 0);
    send(sockRod, hashAndCipher.c_str(), hashAndCipher.length(), 0);

    byte iv[CryptoPP::AES::BLOCKSIZE];

    // Get response from host
    read(sockRod, &payloadLength, sizeof(payloadLength));
    read(sockRod, iv, sizeof(iv));
    char buffer2[payloadLength] = {0};
    size_t r2 = read(sockRod, buffer2, sizeof(buffer2));

    close(sockRod);

    // Decrypt Message
    std::string message(buffer2, r2);
    CryptoPP::SecByteBlock vec(iv, sizeof(iv));
    if (message.length() <= 32)
    {
        logWarning("Error decrypting message, did not recieve all data");
    }
    else
    {
        std::string hash = message.substr(0, 32);
        std::string cipher = message.substr(32);
        AESPacket aesp = {cipher, hash, vec};
        std::string sendStr = decrypt(shared.data(), aesp);

        if (sendStr.find("HTTP/1.1 200 OK\r\n") != 0)
        {
            int f = sendStr.find("HTTP/1.1 ") + 9;
            int f1 = sendStr.find("\r\n");
            std::string error = sendStr.substr(f, f1);
            logError("A Lightning Rod returned error (" + error + ") retrying...");

            if (++peers.at(host).infractions > 3 && config.getHosts().size() > 1)
            {
                config.getHosts().erase(config.getHosts().begin() + i);
            }

            return handleRequest(sock, i);
        }

        // Send response back to lnd
        send(sock, sendStr.c_str(), sendStr.length(), 0);
    }

    close(sock);
}
