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

#include "APIBaseModel.h"
#include "APIERC1155CollectionItem.h"
#include "APIERC721CollectionItem.h"

namespace ImmutablezkEVMAPI
{

/*
 * APIAssetCollectionItem
 *
 * 
 */
class IMMUTABLEZKEVMAPI_API APIAssetCollectionItem : public Model
{
public:
    virtual ~APIAssetCollectionItem() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	enum class TypeEnum
	{
		ERC721COLLECTION,
		ERC1155COLLECTION,
  	};

	static FString EnumToString(const TypeEnum& EnumValue);
	static bool EnumFromString(const FString& EnumAsString, TypeEnum& EnumValue);
	/* Token type user is offering, which in this case is ERC721 */
	TypeEnum Type;
	/* Address of ERC1155 collection */
	FString ContractAddress;
	/* A string representing the price at which the user is willing to sell the token. This value is provided in the smallest unit of the token (e.g., wei for Ethereum). */
	FString Amount;
};

}
