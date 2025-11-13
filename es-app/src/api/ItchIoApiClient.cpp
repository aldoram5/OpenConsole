#include "ItchIoApiClient.h"
#include "Log.h"
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <fstream>
#include <sstream>

namespace OpenConsole
{
	ItchIoApiClient::ItchIoApiClient()
		: mBaseUrl("https://api.itch.io"),
		  mUserAgent("OpenConsole/1.0")
	{
		// Initialize curl globally (should be done once)
		static bool curlInitialized = false;
		if (!curlInitialized)
		{
			curl_global_init(CURL_GLOBAL_DEFAULT);
			curlInitialized = true;
		}
	}

	ItchIoApiClient::~ItchIoApiClient()
	{
		// Note: We don't call curl_global_cleanup() as it's a global resource
	}

	void ItchIoApiClient::setApiKey(const std::string& apiKey)
	{
		mApiKey = apiKey;
	}

	bool ItchIoApiClient::testApiKey()
	{
		if (mApiKey.empty())
		{
			mLastError = "API key not set";
			return false;
		}

		ItchIoProfile profile;
		return getProfile(profile);
	}

	bool ItchIoApiClient::getProfile(ItchIoProfile& profile)
	{
		std::string response = httpGet("/profile");
		if (response.empty())
			return false;

		rapidjson::Document doc;
		if (!parseJsonResponse(response, doc))
			return false;

		// Parse profile
		if (!doc.HasMember("user") || !doc["user"].IsObject())
		{
			mLastError = "Invalid profile response: missing user object";
			return false;
		}

		const auto& user = doc["user"];

		if (user.HasMember("id") && user["id"].IsInt())
			profile.id = user["id"].GetInt();

		if (user.HasMember("username") && user["username"].IsString())
			profile.username = user["username"].GetString();

		if (user.HasMember("display_name") && user["display_name"].IsString())
			profile.displayName = user["display_name"].GetString();

		if (user.HasMember("url") && user["url"].IsString())
			profile.url = user["url"].GetString();

		if (user.HasMember("cover_url") && user["cover_url"].IsString())
			profile.coverUrl = user["cover_url"].GetString();

		Log::write(LogInfo, "ItchIoApiClient: Fetched profile for user: " + profile.username);
		return true;
	}

	std::vector<ItchIoGame> ItchIoApiClient::getOwnedGames()
	{
		std::vector<ItchIoGame> games;

		std::string response = httpGet("/profile/owned-keys");
		if (response.empty())
			return games;

		rapidjson::Document doc;
		if (!parseJsonResponse(response, doc))
			return games;

		if (!doc.HasMember("owned_keys") || !doc["owned_keys"].IsArray())
		{
			mLastError = "Invalid owned-keys response: missing owned_keys array";
			return games;
		}

		const auto& ownedKeys = doc["owned_keys"].GetArray();

		for (const auto& key : ownedKeys)
		{
			if (!key.IsObject() || !key.HasMember("game"))
				continue;

			const auto& gameObj = key["game"];
			if (!gameObj.IsObject())
				continue;

			ItchIoGame game;

			// Parse game information
			if (gameObj.HasMember("id") && gameObj["id"].IsInt())
				game.id = gameObj["id"].GetInt();

			if (gameObj.HasMember("title") && gameObj["title"].IsString())
				game.title = gameObj["title"].GetString();

			if (gameObj.HasMember("url") && gameObj["url"].IsString())
				game.url = gameObj["url"].GetString();

			if (gameObj.HasMember("cover_url") && gameObj["cover_url"].IsString())
				game.coverUrl = gameObj["cover_url"].GetString();

			if (gameObj.HasMember("short_text") && gameObj["short_text"].IsString())
				game.shortText = gameObj["short_text"].GetString();

			if (gameObj.HasMember("created_at") && gameObj["created_at"].IsString())
				game.createdAt = gameObj["created_at"].GetString();

			if (gameObj.HasMember("published_at") && gameObj["published_at"].IsString())
				game.publishedAt = gameObj["published_at"].GetString();

			// Get download information from key
			if (key.HasMember("downloads") && key["downloads"].IsArray())
			{
				const auto& downloads = key["downloads"].GetArray();
				if (downloads.Size() > 0)
				{
					// Use first available download
					const auto& download = downloads[0];
					if (download.IsObject())
					{
						if (download.HasMember("id") && download["id"].IsInt())
							game.uploadId = download["id"].GetInt();

						if (download.HasMember("filename") && download["filename"].IsString())
							game.uploadFilename = download["filename"].GetString();

						if (download.HasMember("size") && download["size"].IsUint64())
							game.uploadSize = download["size"].GetUint64();
					}
				}
			}

			// Only add games that have downloadable content
			if (game.uploadId > 0)
			{
				games.push_back(game);
			}
		}

		Log::write(LogInfo, "ItchIoApiClient: Fetched " + std::to_string(games.size()) + " owned games");
		return games;
	}

	std::string ItchIoApiClient::getDownloadUrl(int uploadId)
	{
		std::string endpoint = "/uploads/" + std::to_string(uploadId) + "/download";
		std::string response = httpGet(endpoint);
		if (response.empty())
			return "";

		rapidjson::Document doc;
		if (!parseJsonResponse(response, doc))
			return "";

		if (!doc.HasMember("url") || !doc["url"].IsString())
		{
			mLastError = "Invalid download response: missing url";
			return "";
		}

		std::string downloadUrl = doc["url"].GetString();
		Log::write(LogInfo, "ItchIoApiClient: Got download URL for upload " + std::to_string(uploadId));
		return downloadUrl;
	}

	bool ItchIoApiClient::downloadFile(const std::string& url, const std::string& outputPath,
		DownloadProgressCallback progressCallback)
	{
		mProgressCallback = progressCallback;

		CURL* curl = curl_easy_init();
		if (!curl)
		{
			mLastError = "Failed to initialize CURL";
			return false;
		}

		// Open output file
		std::ofstream outFile(outputPath, std::ios::binary);
		if (!outFile.is_open())
		{
			mLastError = "Failed to open output file: " + outputPath;
			curl_easy_cleanup(curl);
			return false;
		}

		// Set up CURL options
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, mUserAgent.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

		// Write callback
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);

		// Progress callback
		if (progressCallback)
		{
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		}

		// Perform request
		CURLcode res = curl_easy_perform(curl);

		// Check response code
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		curl_easy_cleanup(curl);
		outFile.close();

		if (res != CURLE_OK)
		{
			mLastError = "CURL error: " + std::string(curl_easy_strerror(res));
			return false;
		}

		if (httpCode != 200)
		{
			mLastError = "HTTP error " + std::to_string(httpCode);
			return false;
		}

		Log::write(LogInfo, "ItchIoApiClient: Downloaded file to " + outputPath);
		return true;
	}

	std::string ItchIoApiClient::httpGet(const std::string& endpoint)
	{
		std::string url = mBaseUrl + endpoint;

		// Add API key to URL
		if (!mApiKey.empty())
		{
			char separator = (endpoint.find('?') != std::string::npos) ? '&' : '?';
			url += separator + std::string("api_key=") + mApiKey;
		}

		return httpGetUrl(url);
	}

	std::string ItchIoApiClient::httpGetUrl(const std::string& url)
	{
		CURL* curl = curl_easy_init();
		if (!curl)
		{
			mLastError = "Failed to initialize CURL";
			return "";
		}

		std::string response;

		// Set up CURL options
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, mUserAgent.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		// Perform request
		CURLcode res = curl_easy_perform(curl);

		// Check response code
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		curl_easy_cleanup(curl);

		if (res != CURLE_OK)
		{
			mLastError = "CURL error: " + std::string(curl_easy_strerror(res));
			return "";
		}

		if (httpCode != 200)
		{
			mLastError = "HTTP error " + std::to_string(httpCode) + ": " + response;
			return "";
		}

		return response;
	}

	bool ItchIoApiClient::parseJsonResponse(const std::string& json, rapidjson::Document& doc)
	{
		doc.Parse(json.c_str());

		if (doc.HasParseError())
		{
			mLastError = "JSON parse error at offset " + std::to_string(doc.GetErrorOffset());
			Log::write(LogError, "ItchIoApiClient: " + mLastError);
			return false;
		}

		// Check for API errors
		if (doc.HasMember("errors") && doc["errors"].IsArray())
		{
			const auto& errors = doc["errors"].GetArray();
			if (errors.Size() > 0 && errors[0].IsString())
			{
				mLastError = "API error: " + std::string(errors[0].GetString());
				Log::write(LogError, "ItchIoApiClient: " + mLastError);
				return false;
			}
		}

		return true;
	}

	size_t ItchIoApiClient::curlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
	{
		size_t realsize = size * nmemb;

		if (std::ofstream* file = static_cast<std::ofstream*>(userp))
		{
			file->write(static_cast<char*>(contents), realsize);
		}
		else if (std::string* str = static_cast<std::string*>(userp))
		{
			str->append(static_cast<char*>(contents), realsize);
		}

		return realsize;
	}

	int ItchIoApiClient::curlProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
		curl_off_t ultotal, curl_off_t ulnow)
	{
		(void)ultotal;
		(void)ulnow;

		ItchIoApiClient* client = static_cast<ItchIoApiClient*>(clientp);
		if (client && client->mProgressCallback && dltotal > 0)
		{
			client->mProgressCallback(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
		}

		return 0; // Continue download
	}

} // namespace OpenConsole
