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
#include "APIRecordStringTypedDataFieldValueInner.h"
#include "APITypedDataDomain.h"

namespace ImmutableOrderbook
{

/*
 * APISignableActionMessage
 *
 * 
 */
class IMMUTABLEORDERBOOK_API APISignableActionMessage : public Model
{
public:
    virtual ~APISignableActionMessage() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	TOptional<APITypedDataDomain> Domain;
	TOptional<TMap<FString, TArray<APIRecordStringTypedDataFieldValueInner>>> Types;
	TOptional<TMap<FString, TSharedPtr<FJsonValue>>> Message;
	TOptional<FString> PrimaryType;
};

}
