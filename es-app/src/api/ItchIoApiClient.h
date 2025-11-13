#pragma once
#ifndef ES_APP_API_ITCHIO_API_CLIENT_H
#define ES_APP_API_ITCHIO_API_CLIENT_H

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace OpenConsole
{
	// itch.io game information from API
	struct ItchIoGame
	{
		int id;                     // itch.io game ID
		std::string title;
		std::string url;
		std::string coverUrl;
		std::string shortText;      // Short description
		std::string createdAt;
		std::string publishedAt;

		// Upload information (for downloading)
		int uploadId;
		std::string uploadFilename;
		size_t uploadSize;
		std::string downloadUrl;    // Temporary download URL

		ItchIoGame() : id(0), uploadId(0), uploadSize(0) {}
	};

	// User profile information
	struct ItchIoProfile
	{
		int id;
		std::string username;
		std::string displayName;
		std::string url;
		std::string coverUrl;

		ItchIoProfile() : id(0) {}
	};

	// Download progress callback
	// Parameters: bytesDownloaded, totalBytes
	using DownloadProgressCallback = std::function<void(size_t, size_t)>;

	/**
	 * ItchIoApiClient - HTTP client for itch.io API
	 *
	 * Handles all communication with the itch.io API including:
	 * - User authentication (API key based)
	 * - Profile retrieval
	 * - Owned games library
	 * - Download URL generation
	 *
	 * API Documentation: https://itch.io/docs/api/overview
	 */
	class ItchIoApiClient
	{
	public:
		ItchIoApiClient();
		~ItchIoApiClient();

		// Set API key for authentication
		void setApiKey(const std::string& apiKey);

		// Get current API key
		std::string getApiKey() const { return mApiKey; }

		// Test if API key is valid
		// Returns true if the key works and can fetch profile
		bool testApiKey();

		// Get user profile
		// Returns true on success, profile will be filled
		bool getProfile(ItchIoProfile& profile);

		// Get owned games (purchased/claimed games)
		// Returns vector of games, empty on error
		std::vector<ItchIoGame> getOwnedGames();

		// Get download URL for a specific upload
		// uploadId: The upload ID from game information
		// Returns download URL (temporary, expires after some time)
		std::string getDownloadUrl(int uploadId);

		// Download a file from URL to local path
		// url: The download URL (from getDownloadUrl)
		// outputPath: Where to save the file
		// progressCallback: Optional callback for progress updates
		// Returns true on success
		bool downloadFile(const std::string& url, const std::string& outputPath,
			DownloadProgressCallback progressCallback = nullptr);

		// Get last error message
		std::string getLastError() const { return mLastError; }

		// Set API base URL (default: https://api.itch.io)
		void setBaseUrl(const std::string& baseUrl) { mBaseUrl = baseUrl; }

		// Set user agent string
		void setUserAgent(const std::string& userAgent) { mUserAgent = userAgent; }

	private:
		// Make HTTP GET request
		// endpoint: API endpoint (e.g., "/profile")
		// Returns response body, empty on error
		std::string httpGet(const std::string& endpoint);

		// Make HTTP GET request with full URL
		std::string httpGetUrl(const std::string& url);

		// Parse JSON response from itch.io API
		bool parseJsonResponse(const std::string& json, rapidjson::Document& doc);

		// CURL write callback
		static size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

		// CURL progress callback
		static int curlProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
			curl_off_t ultotal, curl_off_t ulnow);

		std::string mApiKey;
		std::string mBaseUrl;
		std::string mUserAgent;
		std::string mLastError;

		// Progress callback for downloads
		DownloadProgressCallback mProgressCallback;
	};

} // namespace OpenConsole

#endif // ES_APP_API_ITCHIO_API_CLIENT_H
