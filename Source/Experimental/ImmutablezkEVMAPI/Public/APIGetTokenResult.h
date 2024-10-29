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
#include "APIToken.h"

namespace ImmutablezkEVMAPI
{

/*
 * APIGetTokenResult
 *
 * Single Token
 */
class IMMUTABLEZKEVMAPI_API APIGetTokenResult : public Model
{
public:
    virtual ~APIGetTokenResult() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	APIToken Result;
};

}
