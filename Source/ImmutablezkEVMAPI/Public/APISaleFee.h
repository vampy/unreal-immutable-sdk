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
 * APISaleFee
 *
 * 
 */
class IMMUTABLEZKEVMAPI_API APISaleFee : public Model
{
public:
    virtual ~APISaleFee() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	/* Fee payable to recipient upon settlement */
	TOptional<FString> Amount;
	enum class TypeEnum
	{
		Royalty,
  	};

	static FString EnumToString(const TypeEnum& EnumValue);
	static bool EnumFromString(const FString& EnumAsString, TypeEnum& EnumValue);
	/* Fee type */
	TOptional<TypeEnum> Type;
	/* Wallet address of fee recipient */
	TOptional<FString> Recipient;
};

}
