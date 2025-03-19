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

namespace ImmutablezkEVMAPI
{

/*
 * APIERC721Item
 *
 * 
 */
class IMMUTABLEZKEVMAPI_API APIERC721Item : public Model
{
public:
    virtual ~APIERC721Item() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	enum class TypeEnum
	{
		ERC721,
  	};

	static FString EnumToString(const TypeEnum& EnumValue);
	static bool EnumFromString(const FString& EnumAsString, TypeEnum& EnumValue);
	/* Token type user is offering, which in this case is ERC721 */
	TypeEnum Type;
	/* Address of ERC721 token */
	FString ContractAddress;
	/* ID of ERC721 token */
	FString TokenId;
};

}
