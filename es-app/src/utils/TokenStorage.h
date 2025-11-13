#pragma once
#ifndef ES_APP_UTILS_TOKEN_STORAGE_H
#define ES_APP_UTILS_TOKEN_STORAGE_H

#include <string>
#include <map>

namespace OpenConsole
{
	/**
	 * TokenStorage - Secure storage for authentication tokens and credentials
	 *
	 * Stores tokens encrypted on disk using AES-256-CBC encryption.
	 * Storage location: ~/.openconsole/credentials.enc
	 *
	 * Thread-safe: No (call from main thread only)
	 */
	class TokenStorage
	{
	public:
		static TokenStorage& getInstance();

		// Initialize token storage (creates directory if needed)
		bool initialize();

		// Store a token for a service
		// serviceName: identifier (e.g., "itch_io")
		// token: the authentication token to store
		// Returns true on success
		bool storeToken(const std::string& serviceName, const std::string& token);

		// Retrieve a token for a service
		// Returns empty string if not found
		std::string getToken(const std::string& serviceName);

		// Check if a token exists for a service
		bool hasToken(const std::string& serviceName) const;

		// Remove a token for a service
		bool removeToken(const std::string& serviceName);

		// Clear all stored tokens
		bool clearAll();

		// Get last error message
		std::string getLastError() const { return mLastError; }

	private:
		TokenStorage();
		~TokenStorage();
		TokenStorage(const TokenStorage&) = delete;
		TokenStorage& operator=(const TokenStorage&) = delete;

		// Load tokens from encrypted file
		bool loadFromFile();

		// Save tokens to encrypted file
		bool saveToFile();

		// Get storage file path
		std::string getStorageFilePath() const;

		// Get encryption key (derived from machine-specific data)
		std::string getEncryptionKey() const;

		// Encrypt data using AES-256-CBC
		std::string encrypt(const std::string& plaintext) const;

		// Decrypt data using AES-256-CBC
		std::string decrypt(const std::string& ciphertext) const;

		std::map<std::string, std::string> mTokens;
		std::string mLastError;
		bool mInitialized;
	};

} // namespace OpenConsole

#endif // ES_APP_UTILS_TOKEN_STORAGE_H
