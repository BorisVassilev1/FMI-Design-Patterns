#pragma once
#include <iostream>
#include <string>
#include <istream>
#include <openssl/evp.h>
#include "factory.hpp"
#include "utils.hpp"

class ChecksumCalculator {
   public:
	virtual std::string calculate(std::istream &) = 0;
	virtual ~ChecksumCalculator()				  = default;
};

template <const EVP_MD *(*alg)()>
class OpenSSLChecksumCalculator : public ChecksumCalculator {
   public:
	virtual std::string calculate(std::istream &input) override {
		EVP_get_digestbyname("md5");
		EVP_MD_CTX	 *context = EVP_MD_CTX_new();
		const EVP_MD *md	  = alg();
		unsigned char md_value[EVP_MAX_MD_SIZE];
		unsigned int  md_len;

		EVP_DigestInit(context, md);
		char buffer[1024];
		while (input.read(buffer, sizeof(buffer))) {
			EVP_DigestUpdate(context, buffer, input.gcount());
		}
		EVP_DigestUpdate(context, buffer, input.gcount());
		EVP_DigestFinal(context, md_value, &md_len);
		EVP_MD_CTX_free(context);
		std::string result;
		result.resize(2 * md_len);
		for (unsigned int i = 0; i < md_len; i++) {
			sprintf(result.data() + 2 * i, "%02x", md_value[i]);
		}
		return result;
	}
};

using ChecksumCalculatorFactory = Factory<ChecksumCalculator>;

// copy-pasta from openssl/evp.h
#ifndef OPENSSL_NO_MD2
using MD2ChecksumCalculator = OpenSSLChecksumCalculator<EVP_md2>;
#endif
#ifndef OPENSSL_NO_MD4
using MD4ChecksumCalculator = OpenSSLChecksumCalculator<EVP_md4>;
#endif
#ifndef OPENSSL_NO_MD5
using MD5ChecksumCalculator		 = OpenSSLChecksumCalculator<EVP_md5>;
using MD5_SHA1ChecksumCalculator = OpenSSLChecksumCalculator<EVP_md5_sha1>;
#endif
#ifndef OPENSSL_NO_BLAKE2
using BLAKE2B512ChecksumCalculator = OpenSSLChecksumCalculator<EVP_blake2b512>;
using BLAKE2S256ChecksumCalculator = OpenSSLChecksumCalculator<EVP_blake2s256>;
#endif
using SHA1ChecksumCalculator	   = OpenSSLChecksumCalculator<EVP_sha1>;
using SHA224ChecksumCalculator	   = OpenSSLChecksumCalculator<EVP_sha224>;
using SHA256ChecksumCalculator	   = OpenSSLChecksumCalculator<EVP_sha256>;
using SHA384ChecksumCalculator	   = OpenSSLChecksumCalculator<EVP_sha384>;
using SHA512ChecksumCalculator	   = OpenSSLChecksumCalculator<EVP_sha512>;
using SHA512_224ChecksumCalculator = OpenSSLChecksumCalculator<EVP_sha512_224>;
using SHA512_256ChecksumCalculator = OpenSSLChecksumCalculator<EVP_sha512_256>;
using SHA3_224ChecksumCalculator   = OpenSSLChecksumCalculator<EVP_sha3_224>;
using SHA3_256ChecksumCalculator   = OpenSSLChecksumCalculator<EVP_sha3_256>;
using SHA3_384ChecksumCalculator   = OpenSSLChecksumCalculator<EVP_sha3_384>;
using SHA3_512ChecksumCalculator   = OpenSSLChecksumCalculator<EVP_sha3_512>;
using SHAKE128ChecksumCalculator   = OpenSSLChecksumCalculator<EVP_shake128>;
using SHAKE256ChecksumCalculator   = OpenSSLChecksumCalculator<EVP_shake256>;

#ifndef OPENSSL_NO_MDC2
using MDC2ChecksumCalculator = OpenSSLChecksumCalculator<EVP_mdc2>;
#endif
#ifndef OPENSSL_NO_RMD160
using RIPEMD160ChecksumCalculator = OpenSSLChecksumCalculator<EVP_ripemd160>;
#endif
#ifndef OPENSSL_NO_WHIRLPOOL
using WHIRLPOOLChecksumCalculator = OpenSSLChecksumCalculator<EVP_whirlpool>;
#endif
#ifndef OPENSSL_NO_SM3
using SM3ChecksumCalculator = OpenSSLChecksumCalculator<EVP_sm3>;
#endif

JOB(OpenSSLChecksumCalculator, {
#ifndef OPENSSL_NO_MD2
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_md2>>("md2");
#endif
#ifndef OPENSSL_NO_MD4
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_md4>>("md4");
#endif
#ifndef OPENSSL_NO_MD5
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_md5>>("md5");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_md5_sha1>>("md5_sha1");
#endif
#ifndef OPENSSL_NO_BLAKE2
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_blake2b512>>("blake2b512");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_blake2s256>>("blake2s256");
#endif
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha1>>("sha1");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha224>>("sha224");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha256>>("sha256");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha384>>("sha384");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha512>>("sha512");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha512_224>>("sha512_224");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha512_256>>("sha512_256");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha3_224>>("sha3_224");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha3_256>>("sha3_256");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha3_384>>("sha3_384");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sha3_512>>("sha3_512");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_shake128>>("shake128");
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_shake256>>("shake256");

#ifndef OPENSSL_NO_MDC2
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_mdc2>>("mdc2");
#endif
#ifndef OPENSSL_NO_RMD160
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_ripemd160>>("ripemd160");
#endif
#ifndef OPENSSL_NO_WHIRLPOOL
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_whirlpool>>("whirlpool");
#endif
#ifndef OPENSSL_NO_SM3
	ChecksumCalculatorFactory::instance().registerType<OpenSSLChecksumCalculator<EVP_sm3>>("sm3");
#endif
});
