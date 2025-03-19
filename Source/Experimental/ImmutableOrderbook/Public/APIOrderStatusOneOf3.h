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

namespace ImmutableOrderbook
{

/*
 * APIOrderStatusOneOf3
 *
 * 
 */
class IMMUTABLEORDERBOOK_API APIOrderStatusOneOf3 : public Model
{
public:
    virtual ~APIOrderStatusOneOf3() {}
	bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) final;
	void WriteJson(JsonWriter& Writer) const final;

	enum class NameEnum
	{
		Filled,
  	};

	static FString EnumToString(const NameEnum& EnumValue);
	static bool EnumFromString(const FString& EnumAsString, NameEnum& EnumValue);
	/* A terminal order status indicating that an order has been fulfilled. */
	TOptional<NameEnum> Name;
};

}
