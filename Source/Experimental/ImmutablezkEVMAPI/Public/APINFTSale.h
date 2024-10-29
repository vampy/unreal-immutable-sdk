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
#include "APISalePayment.h"

namespace ImmutablezkEVMAPI
{

/*
 * APINFTSale
 *
 * The NFT Sale activity details
 */
class IMMUTABLEZKEVMAPI_API APINFTSale : public Model
{
public:
    virtual ~APINFTSale() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	/* The id of order */
	FString OrderId;
	/* The account address of buyer */
	FString To;
	/* The account address of seller */
	FString From;
	TArray<APIActivityNFT> Asset;
	APISalePayment Payment;
};

}
