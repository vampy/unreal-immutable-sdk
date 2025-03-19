/**
 * TS SDK API
 * running ts sdk as an api
 *
 * OpenAPI spec version: 1.0.0
 * Contact: contact@immutable.com
 *
 * NOTE: This class is auto generated by OpenAPI Generator
 * https://github.com/OpenAPITools/openapi-generator
 * Do not edit the class manually.
 */

#pragma once

#include "APIBaseModel.h"
#include "APIPrepareListingRequestBuy.h"
#include "APIPrepareListingRequestSell.h"

namespace ImmutableOrderbook
{

/*
 * APIPrepareListingRequest
 *
 * 
 */
class IMMUTABLEORDERBOOK_API APIPrepareListingRequest : public Model
{
public:
    virtual ~APIPrepareListingRequest() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	APIPrepareListingRequestBuy Buy;
	FString MakerAddress;
	TOptional<FDateTime> OrderExpiry;
	APIPrepareListingRequestSell Sell;
};

}
