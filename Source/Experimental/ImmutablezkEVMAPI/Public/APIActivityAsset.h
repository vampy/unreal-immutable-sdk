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
#include "APIActivityNFT.h"
#include "APIActivityToken.h"
#include "APITokenContractType.h"

namespace ImmutablezkEVMAPI
{

/*
 * APIActivityAsset
 *
 * The contract and asset details for this activity
 */
class IMMUTABLEZKEVMAPI_API APIActivityAsset : public Model
{
public:
    virtual ~APIActivityAsset() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	APITokenContractType ContractType;
	/* The contract address */
	FString ContractAddress;
	/* An `uint256` token id as string */
	FString TokenId;
	/* (deprecated - will never be filled, use amount on Activity instead) The amount of tokens exchanged */
	FString Amount;
};

}
