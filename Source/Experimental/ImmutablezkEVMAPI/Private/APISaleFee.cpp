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

#include "APISaleFee.h"

#include "ImmutablezkEVMAPIModule.h"
#include "APIHelpers.h"

#include "Templates/SharedPointer.h"

namespace ImmutablezkEVMAPI
{

inline FString ToString(const APISaleFee::TypeEnum& Value)
{
	switch (Value)
	{
	case APISaleFee::TypeEnum::Royalty:
		return TEXT("ROYALTY");
	}

	UE_LOG(LogImmutablezkEVMAPI, Error, TEXT("Invalid APISaleFee::TypeEnum Value (%d)"), (int)Value);
	return TEXT("");
}

FString APISaleFee::EnumToString(const APISaleFee::TypeEnum& EnumValue)
{
	return ToString(EnumValue);
}

inline bool FromString(const FString& EnumAsString, APISaleFee::TypeEnum& Value)
{
	static TMap<FString, APISaleFee::TypeEnum> StringToEnum = { 
		{ TEXT("ROYALTY"), APISaleFee::TypeEnum::Royalty }, };

	const auto Found = StringToEnum.Find(EnumAsString);
	if(Found)
		Value = *Found;

	return Found != nullptr;
}

bool APISaleFee::EnumFromString(const FString& EnumAsString, APISaleFee::TypeEnum& EnumValue)
{
	return FromString(EnumAsString, EnumValue);
}

inline void WriteJsonValue(JsonWriter& Writer, const APISaleFee::TypeEnum& Value)
{
	WriteJsonValue(Writer, ToString(Value));
}

inline bool TryGetJsonValue(const TSharedPtr<FJsonValue>& JsonValue, APISaleFee::TypeEnum& Value)
{
	FString TmpValue;
	if (JsonValue->TryGetString(TmpValue))
	{
		if(FromString(TmpValue, Value))
			return true;
	}
	return false;
}

void APISaleFee::WriteJson(JsonWriter& Writer) const
{
	Writer->WriteObjectStart();
	if (Amount.IsSet())
	{
		Writer->WriteIdentifierPrefix(TEXT("amount")); WriteJsonValue(Writer, Amount.GetValue());
	}
	if (Type.IsSet())
	{
		Writer->WriteIdentifierPrefix(TEXT("type")); WriteJsonValue(Writer, Type.GetValue());
	}
	if (Recipient.IsSet())
	{
		Writer->WriteIdentifierPrefix(TEXT("recipient")); WriteJsonValue(Writer, Recipient.GetValue());
	}
	Writer->WriteObjectEnd();
}

bool APISaleFee::FromJson(const TSharedPtr<FJsonValue>& JsonValue)
{
	const TSharedPtr<FJsonObject>* Object;
	if (!JsonValue->TryGetObject(Object))
		return false;

	bool ParseSuccess = true;

	ParseSuccess &= TryGetJsonValue(*Object, TEXT("amount"), Amount);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("type"), Type);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("recipient"), Recipient);

	return ParseSuccess;
}

}
