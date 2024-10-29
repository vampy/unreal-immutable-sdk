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
#include "APINFTContractType.h"

namespace ImmutablezkEVMAPI
{

/*
 * APIActivityNFT
 *
 * 
 */
class IMMUTABLEZKEVMAPI_API APIActivityNFT : public Model
{
public:
    virtual ~APIActivityNFT() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	APINFTContractType ContractType;
	/* The token contract address */
	FString ContractAddress;
	/* An `uint256` token id as string */
	FString TokenId;
	/* (deprecated - will never be filled, use amount on Activity instead) The amount of tokens exchanged */
	FString Amount;
};

}
