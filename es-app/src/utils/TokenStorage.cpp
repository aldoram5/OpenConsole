#include "TokenStorage.h"
#include "Log.h"
#include "platform.h"
#include "utils/FileSystemUtil.h"
#include <fstream>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <unistd.h>

namespace OpenConsole
{
	// AES-256 requires 32 byte key
	constexpr size_t AES_KEY_SIZE = 32;
	constexpr size_t AES_IV_SIZE = 16;

	TokenStorage& TokenStorage::getInstance()
	{
		static TokenStorage instance;
		return instance;
	}

	TokenStorage::TokenStorage()
		: mInitialized(false)
	{
	}

	TokenStorage::~TokenStorage()
	{
		// Clear tokens from memory
		mTokens.clear();
	}

	bool TokenStorage::initialize()
	{
		if (mInitialized)
			return true;

		// Create OpenConsole directory if it doesn't exist
		std::string ocDir = Utils::FileSystem::getHomePath() + "/.openconsole";
		struct stat st;
		if (stat(ocDir.c_str(), &st) != 0)
		{
			if (mkdir(ocDir.c_str(), 0700) != 0)
			{
				mLastError = "Failed to create OpenConsole directory: " + ocDir;
				LOG(LogError) << "TokenStorage: " + mLastError;
				return false;
			}
		}

		// Load existing tokens
		loadFromFile();

		mInitialized = true;
		LOG(LogInfo) << "TokenStorage: Initialized successfully";
		return true;
	}

	bool TokenStorage::storeToken(const std::string& serviceName, const std::string& token)
	{
		if (!mInitialized)
		{
			mLastError = "TokenStorage not initialized";
			return false;
		}

		mTokens[serviceName] = token;

		if (!saveToFile())
		{
			// Rollback on failure
			mTokens.erase(serviceName);
			return false;
		}

		LOG(LogInfo) << "TokenStorage: Stored token for service: " + serviceName;
		return true;
	}

	std::string TokenStorage::getToken(const std::string& serviceName)
	{
		if (!mInitialized)
		{
			mLastError = "TokenStorage not initialized";
			return "";
		}

		auto it = mTokens.find(serviceName);
		if (it != mTokens.end())
			return it->second;

		return "";
	}

	bool TokenStorage::hasToken(const std::string& serviceName) const
	{
		return mTokens.find(serviceName) != mTokens.end();
	}

	bool TokenStorage::removeToken(const std::string& serviceName)
	{
		if (!mInitialized)
		{
			mLastError = "TokenStorage not initialized";
			return false;
		}

		auto it = mTokens.find(serviceName);
		if (it == mTokens.end())
			return true; // Already removed

		std::string backup = it->second;
		mTokens.erase(it);

		if (!saveToFile())
		{
			// Rollback on failure
			mTokens[serviceName] = backup;
			return false;
		}

		LOG(LogInfo) << "TokenStorage: Removed token for service: " + serviceName;
		return true;
	}

	bool TokenStorage::clearAll()
	{
		if (!mInitialized)
		{
			mLastError = "TokenStorage not initialized";
			return false;
		}

		mTokens.clear();

		if (!saveToFile())
			return false;

		LOG(LogInfo) << "TokenStorage: Cleared all tokens";
		return true;
	}

	std::string TokenStorage::getStorageFilePath() const
	{
		return Utils::FileSystem::getHomePath() + "/.openconsole/credentials.enc";
	}

	std::string TokenStorage::getEncryptionKey() const
	{
		// Generate encryption key from machine-specific data
		// This is not perfectly secure but provides reasonable protection
		// for locally stored tokens

		std::string keyMaterial;

		// Get hostname
		char hostname[256];
		if (gethostname(hostname, sizeof(hostname)) == 0)
			keyMaterial += hostname;

		// Add user home path
		keyMaterial += Utils::FileSystem::getHomePath();

		// Add a fixed salt specific to OpenConsole
		keyMaterial += "OpenConsole-ItchIO-Token-Storage-v1";

		// Hash the key material to get a 32-byte key
		unsigned char hash[SHA256_DIGEST_LENGTH];
		SHA256(reinterpret_cast<const unsigned char*>(keyMaterial.c_str()),
			keyMaterial.length(), hash);

		return std::string(reinterpret_cast<char*>(hash), SHA256_DIGEST_LENGTH);
	}

	std::string TokenStorage::encrypt(const std::string& plaintext) const
	{
		if (plaintext.empty())
			return "";

		std::string key = getEncryptionKey();

		// Generate random IV
		unsigned char iv[AES_IV_SIZE];
		if (RAND_bytes(iv, AES_IV_SIZE) != 1)
		{
			LOG(LogError) << "TokenStorage: Failed to generate IV";
			return "";
		}

		// Create cipher context
		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		if (!ctx)
		{
			LOG(LogError) << "TokenStorage: Failed to create cipher context";
			return "";
		}

		// Initialize encryption
		if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
			reinterpret_cast<const unsigned char*>(key.c_str()), iv) != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			LOG(LogError) << "TokenStorage: Failed to initialize encryption";
			return "";
		}

		// Allocate output buffer (plaintext size + block size for padding)
		int ciphertext_len = plaintext.length() + AES_BLOCK_SIZE;
		unsigned char* ciphertext = new unsigned char[ciphertext_len];

		int len;
		// Encrypt data
		if (EVP_EncryptUpdate(ctx, ciphertext, &len,
			reinterpret_cast<const unsigned char*>(plaintext.c_str()),
			plaintext.length()) != 1)
		{
			delete[] ciphertext;
			EVP_CIPHER_CTX_free(ctx);
			LOG(LogError) << "TokenStorage: Encryption failed";
			return "";
		}

		int total_len = len;

		// Finalize encryption
		if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1)
		{
			delete[] ciphertext;
			EVP_CIPHER_CTX_free(ctx);
			LOG(LogError) << "TokenStorage: Encryption finalization failed";
			return "";
		}

		total_len += len;
		EVP_CIPHER_CTX_free(ctx);

		// Prepend IV to ciphertext
		std::string result(reinterpret_cast<char*>(iv), AES_IV_SIZE);
		result.append(reinterpret_cast<char*>(ciphertext), total_len);

		delete[] ciphertext;
		return result;
	}

	std::string TokenStorage::decrypt(const std::string& ciphertext) const
	{
		if (ciphertext.length() <= AES_IV_SIZE)
			return "";

		std::string key = getEncryptionKey();

		// Extract IV from beginning of ciphertext
		const unsigned char* iv = reinterpret_cast<const unsigned char*>(ciphertext.c_str());
		const unsigned char* encrypted_data = iv + AES_IV_SIZE;
		int encrypted_len = ciphertext.length() - AES_IV_SIZE;

		// Create cipher context
		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		if (!ctx)
		{
			LOG(LogError) << "TokenStorage: Failed to create cipher context";
			return "";
		}

		// Initialize decryption
		if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
			reinterpret_cast<const unsigned char*>(key.c_str()), iv) != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			LOG(LogError) << "TokenStorage: Failed to initialize decryption";
			return "";
		}

		// Allocate output buffer
		unsigned char* plaintext = new unsigned char[encrypted_len + AES_BLOCK_SIZE];

		int len;
		// Decrypt data
		if (EVP_DecryptUpdate(ctx, plaintext, &len, encrypted_data, encrypted_len) != 1)
		{
			delete[] plaintext;
			EVP_CIPHER_CTX_free(ctx);
			LOG(LogError) << "TokenStorage: Decryption failed";
			return "";
		}

		int total_len = len;

		// Finalize decryption
		if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1)
		{
			delete[] plaintext;
			EVP_CIPHER_CTX_free(ctx);
			LOG(LogError) << "TokenStorage: Decryption finalization failed (possibly wrong key");
			return "";
		}

		total_len += len;
		EVP_CIPHER_CTX_free(ctx);

		std::string result(reinterpret_cast<char*>(plaintext), total_len);
		delete[] plaintext;

		return result;
	}

	bool TokenStorage::loadFromFile()
	{
		std::string filePath = getStorageFilePath();

		// Check if file exists
		struct stat st;
		if (stat(filePath.c_str(), &st) != 0)
		{
			// File doesn't exist yet, that's okay
			LOG(LogInfo) << "TokenStorage: No existing credentials file";
			return true;
		}

		// Read encrypted file
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open())
		{
			mLastError = "Failed to open credentials file for reading";
			LOG(LogWarning) << "TokenStorage: " + mLastError;
			return false;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		std::string encrypted = buffer.str();
		if (encrypted.empty())
		{
			LOG(LogInfo) << "TokenStorage: Credentials file is empty";
			return true;
		}

		// Decrypt file contents
		std::string decrypted = decrypt(encrypted);
		if (decrypted.empty())
		{
			mLastError = "Failed to decrypt credentials file";
			LOG(LogError) << "TokenStorage: " + mLastError;
			return false;
		}

		// Parse JSON
		rapidjson::Document doc;
		doc.Parse(decrypted.c_str());

		if (doc.HasParseError() || !doc.IsObject())
		{
			mLastError = "Failed to parse credentials file (corrupted?)";
			LOG(LogError) << "TokenStorage: " + mLastError;
			return false;
		}

		// Load tokens
		mTokens.clear();
		for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it)
		{
			if (it->value.IsString())
			{
				mTokens[it->name.GetString()] = it->value.GetString();
			}
		}

		LOG(LogInfo) << "TokenStorage: Loaded " + std::to_string(mTokens.size() + " tokens");
		return true;
	}

	bool TokenStorage::saveToFile()
	{
		// Create JSON document
		rapidjson::Document doc;
		doc.SetObject();
		auto& allocator = doc.GetAllocator();

		for (const auto& pair : mTokens)
		{
			rapidjson::Value key(pair.first.c_str(), allocator);
			rapidjson::Value value(pair.second.c_str(), allocator);
			doc.AddMember(key, value, allocator);
		}

		// Convert to string
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		std::string json = buffer.GetString();

		// Encrypt data
		std::string encrypted = encrypt(json);
		if (encrypted.empty())
		{
			mLastError = "Failed to encrypt credentials";
			LOG(LogError) << "TokenStorage: " + mLastError;
			return false;
		}

		// Write to file
		std::string filePath = getStorageFilePath();
		std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
		if (!file.is_open())
		{
			mLastError = "Failed to open credentials file for writing";
			LOG(LogError) << "TokenStorage: " + mLastError;
			return false;
		}

		file.write(encrypted.c_str(), encrypted.length());
		file.close();

		// Set restrictive permissions (owner read/write only)
		chmod(filePath.c_str(), 0600);

		LOG(LogInfo) << "TokenStorage: Saved " + std::to_string(mTokens.size() + " tokens");
		return true;
	}

} // namespace OpenConsole
