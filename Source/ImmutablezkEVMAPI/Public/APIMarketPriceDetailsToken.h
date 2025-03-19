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

#include "Misc/TVariant.h"

#include "APIBaseModel.h"
#include "APIMarketPriceERC20Token.h"
#include "APIMarketPriceNativeToken.h"

namespace ImmutablezkEVMAPI
{

/*
 * APIMarketPriceDetailsToken
 *
 * Token details
 */
class IMMUTABLEZKEVMAPI_API APIMarketPriceDetailsToken : public Model
{
public:
    virtual ~APIMarketPriceDetailsToken() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	enum class TypeEnum
	{
		Native,
		ERC20,
  	};

	static FString EnumToString(const TypeEnum& EnumValue);
	static bool EnumFromString(const FString& EnumAsString, TypeEnum& EnumValue);
	/* Token type user is offering, which in this case is the native IMX token */
	TypeEnum Type;
	TVariant<APIMarketPriceERC20Token, APIMarketPriceNativeToken> OneOf;
};

}
