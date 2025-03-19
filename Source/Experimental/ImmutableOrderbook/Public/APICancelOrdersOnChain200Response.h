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
#include "APITransactionAction.h"

namespace ImmutableOrderbook
{

/*
 * APICancelOrdersOnChain200Response
 *
 * Response schema for the cancelOrder endpoint
 */
class IMMUTABLEORDERBOOK_API APICancelOrdersOnChain200Response : public Model
{
public:
    virtual ~APICancelOrdersOnChain200Response() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	TOptional<APITransactionAction> CancellationAction;
};

}
