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
 * APIAPIError500
 *
 * 
 */
class IMMUTABLEZKEVMAPI_API APIAPIError500 : public Model
{
public:
    virtual ~APIAPIError500() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	/* Error Message */
	FString Message;
	/* Link to IMX documentation that can help resolve this error */
	FString Link;
	/* Trace ID of the initial request */
	FString TraceId;
	enum class CodeEnum
	{
		InternalServerError,
  	};

	static FString EnumToString(const CodeEnum& EnumValue);
	static bool EnumFromString(const FString& EnumAsString, CodeEnum& EnumValue);
	/* Error Code */
	CodeEnum Code;
	/* Additional details to help resolve the error */
	TOptional<TSharedPtr<FJsonObject>> Details;
};

}
