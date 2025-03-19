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
#include "APIPage.h"
#include "APIStackBundle.h"

namespace ImmutablezkEVMAPI
{

/*
 * APISearchStacksResult
 *
 * Search stacks result
 */
class IMMUTABLEZKEVMAPI_API APISearchStacksResult : public Model
{
public:
    virtual ~APISearchStacksResult() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	/* List of stack bundles */
	TArray<APIStackBundle> Result;
	APIPage Page;
};

}
