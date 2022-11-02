#include "Common/Network/Crypto.hpp"
#include <openssl/evp.h>

namespace Common::Network
{
	PublicKeyCryptographer::PublicKeyCryptographer()
	{
	}

	auto PublicKeyCryptographer::encrypt(std::vector<uint8_t>& data) const -> void
	{
	}

	auto PublicKeyCryptographer::decryptFromRemote(std::vector<uint8_t>& data) const -> void
	{
	}

	auto PublicKeyCryptographer::decryptFromLocal(std::vector<uint8_t>& data) const -> void
	{
	}

	auto PublicKeyCryptographer::generateKeyPair() -> void
	{
	}

	auto PublicKeyCryptographer::setRemotePublicKey(CipherKey publicKey) -> void
	{
	}

	auto PublicKeyCryptographer::getLocalPublicKey() const -> CipherKey
	{
		return {};
	}

	auto PublicKeyCryptographer::getLocalPrivateKey() const -> CipherKey
	{
		return {};
	}
} // namespace Common::Network