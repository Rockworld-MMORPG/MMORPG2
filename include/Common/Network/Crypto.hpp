#pragma once

#include <memory>

using EVP_PKEY     = struct evp_pkey_st;
using EVP_PKEY_CTX = struct evp_pkey_ctx_st;

namespace Common::Network
{
	/**
	 * \class PublicKeyCryptographer Crypto.hpp "Network/Crypto.hpp"
	 * \brief Encrypts and decrypts messages using public key cryptography techniques
	 */
	class COMMON_API PublicKeyCryptographer
	{
	public:
		static const auto CIPHER_BYTES = std::size_t(32);
		using CipherKey                = std::array<std::uint8_t, CIPHER_BYTES>;

		/**
		 * \brief Construct a new Public Key Cryptographer object
		 *
		 */
		PublicKeyCryptographer();

		/**
		 * \brief Encrypt a byte vector in-place using the cryptographer's local private key
		 *
		 * \param data The byte vector to encrypt
		 */
		auto encrypt(std::vector<uint8_t>& data) const -> void;

		/**
		 * \brief Decrypt a byte vector in-place using the cryptographer's remote public key
		 *
		 * \param data The byte vector to decrypt
		 */
		auto decryptFromRemote(std::vector<uint8_t>& data) const -> void;

		/**
		 * \brief Decrypt a byte vector in-place using the cryptographer's local public key
		 *
		 * \param data The byte vector to decrypt
		 */
		auto decryptFromLocal(std::vector<uint8_t>& data) const -> void;

		/**
		 * \brief Generate a local key-pair to use in public key encryption
		 *
		 */
		auto generateKeyPair() -> void;

		/**
		 * \brief Set the remote public key to decrypt data with
		 *
		 * \param publicKey
		 */
		auto setRemotePublicKey(CipherKey publicKey) -> void;

		/**
		 * \brief Get the local public key
		 */
		[[nodiscard]] auto getLocalPublicKey() const -> CipherKey;

		/**
		 * \brief Get the local private key
		 */
		[[nodiscard]] auto getLocalPrivateKey() const -> CipherKey;

	private:
	};

} // namespace Common::Network