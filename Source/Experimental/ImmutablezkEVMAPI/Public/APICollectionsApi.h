/**
 * Immutable zkEVM API
 * Immutable Multi Rollup API
 *
 * OpenAPI spec version: 1.0.0
 * Contact: support@immutable.com
 *
 * NOTE: This class is auto generated by OpenAPI Generator
 * https://github.com/OpenAPITools/openapi-generator
 * Do not edit the class manually.
 */

#pragma once

#include "CoreMinimal.h"
#include "APIBaseModel.h"

namespace ImmutablezkEVMAPI
{

class IMMUTABLEZKEVMAPI_API APICollectionsApi
{
public:
	APICollectionsApi();
	~APICollectionsApi();

	/* Sets the URL Endpoint.
	* Note: several fallback endpoints can be configured in request retry policies, see Request::SetShouldRetry */
	void SetURL(const FString& Url);

	/* Adds global header params to all requests */
	void AddHeaderParam(const FString& Key, const FString& Value);
	void ClearHeaderParams();

	/* Sets the retry manager to the user-defined retry manager. User must manage the lifetime of the retry manager.
	* If no retry manager is specified and a request needs retries, a default retry manager will be used.
	* See also: Request::SetShouldRetry */
	void SetHttpRetryManager(FHttpRetrySystem::FManager& RetryManager);
	FHttpRetrySystem::FManager& GetHttpRetryManager();

	class GetCollectionRequest;
	class GetCollectionResponse;
	class ListCollectionsRequest;
	class ListCollectionsResponse;
	class ListCollectionsByNFTOwnerRequest;
	class ListCollectionsByNFTOwnerResponse;
	class RefreshCollectionMetadataRequest;
	class RefreshCollectionMetadataResponse;
	
    DECLARE_DELEGATE_OneParam(FGetCollectionDelegate, const GetCollectionResponse&);
    DECLARE_DELEGATE_OneParam(FListCollectionsDelegate, const ListCollectionsResponse&);
    DECLARE_DELEGATE_OneParam(FListCollectionsByNFTOwnerDelegate, const ListCollectionsByNFTOwnerResponse&);
    DECLARE_DELEGATE_OneParam(FRefreshCollectionMetadataDelegate, const RefreshCollectionMetadataResponse&);
    
    FHttpRequestPtr GetCollection(const GetCollectionRequest& Request, const FGetCollectionDelegate& Delegate = FGetCollectionDelegate()) const;
    FHttpRequestPtr ListCollections(const ListCollectionsRequest& Request, const FListCollectionsDelegate& Delegate = FListCollectionsDelegate()) const;
    FHttpRequestPtr ListCollectionsByNFTOwner(const ListCollectionsByNFTOwnerRequest& Request, const FListCollectionsByNFTOwnerDelegate& Delegate = FListCollectionsByNFTOwnerDelegate()) const;
    FHttpRequestPtr RefreshCollectionMetadata(const RefreshCollectionMetadataRequest& Request, const FRefreshCollectionMetadataDelegate& Delegate = FRefreshCollectionMetadataDelegate()) const;
    
private:
    void OnGetCollectionResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FGetCollectionDelegate Delegate) const;
    void OnListCollectionsResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FListCollectionsDelegate Delegate) const;
    void OnListCollectionsByNFTOwnerResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FListCollectionsByNFTOwnerDelegate Delegate) const;
    void OnRefreshCollectionMetadataResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FRefreshCollectionMetadataDelegate Delegate) const;
    
	FHttpRequestRef CreateHttpRequest(const Request& Request) const;
	bool IsValid() const;
	void HandleResponse(FHttpResponsePtr HttpResponse, bool bSucceeded, Response& InOutResponse) const;

	FString Url;
	TMap<FString,FString> AdditionalHeaderParams;
	mutable FHttpRetrySystem::FManager* RetryManager = nullptr;
	mutable TUniquePtr<HttpRetryManager> DefaultRetryManager;
};

}
