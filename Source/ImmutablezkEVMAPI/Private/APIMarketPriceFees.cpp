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

#include "APIMarketPriceFees.h"

#include "ImmutablezkEVMAPIModule.h"
#include "APIHelpers.h"

#include "Templates/SharedPointer.h"

namespace ImmutablezkEVMAPI
{

inline FString ToString(const APIMarketPriceFees::TypeEnum& Value)
{
	switch (Value)
	{
	case APIMarketPriceFees::TypeEnum::Royalty:
		return TEXT("ROYALTY");
	case APIMarketPriceFees::TypeEnum::MakerEcosystem:
		return TEXT("MAKER_ECOSYSTEM");
	case APIMarketPriceFees::TypeEnum::TakerEcosystem:
		return TEXT("TAKER_ECOSYSTEM");
	case APIMarketPriceFees::TypeEnum::Protocol:
		return TEXT("PROTOCOL");
	}

	UE_LOG(LogImmutablezkEVMAPI, Error, TEXT("Invalid APIMarketPriceFees::TypeEnum Value (%d)"), (int)Value);
	return TEXT("");
}

FString APIMarketPriceFees::EnumToString(const APIMarketPriceFees::TypeEnum& EnumValue)
{
	return ToString(EnumValue);
}

inline bool FromString(const FString& EnumAsString, APIMarketPriceFees::TypeEnum& Value)
{
	static TMap<FString, APIMarketPriceFees::TypeEnum> StringToEnum = { 
		{ TEXT("ROYALTY"), APIMarketPriceFees::TypeEnum::Royalty },
		{ TEXT("MAKER_ECOSYSTEM"), APIMarketPriceFees::TypeEnum::MakerEcosystem },
		{ TEXT("TAKER_ECOSYSTEM"), APIMarketPriceFees::TypeEnum::TakerEcosystem },
		{ TEXT("PROTOCOL"), APIMarketPriceFees::TypeEnum::Protocol }, };

	const auto Found = StringToEnum.Find(EnumAsString);
	if(Found)
		Value = *Found;

	return Found != nullptr;
}

bool APIMarketPriceFees::EnumFromString(const FString& EnumAsString, APIMarketPriceFees::TypeEnum& EnumValue)
{
	return FromString(EnumAsString, EnumValue);
}

inline void WriteJsonValue(JsonWriter& Writer, const APIMarketPriceFees::TypeEnum& Value)
{
	WriteJsonValue(Writer, ToString(Value));
}

inline bool TryGetJsonValue(const TSharedPtr<FJsonValue>& JsonValue, APIMarketPriceFees::TypeEnum& Value)
{
	FString TmpValue;
	if (JsonValue->TryGetString(TmpValue))
	{
		if(FromString(TmpValue, Value))
			return true;
	}
	return false;
}

void APIMarketPriceFees::WriteJson(JsonWriter& Writer) const
{
	Writer->WriteObjectStart();
	Writer->WriteIdentifierPrefix(TEXT("amount")); WriteJsonValue(Writer, Amount);
	Writer->WriteIdentifierPrefix(TEXT("type")); WriteJsonValue(Writer, Type);
	Writer->WriteIdentifierPrefix(TEXT("recipient_address")); WriteJsonValue(Writer, RecipientAddress);
	Writer->WriteObjectEnd();
}

bool APIMarketPriceFees::FromJson(const TSharedPtr<FJsonValue>& JsonValue)
{
	const TSharedPtr<FJsonObject>* Object;
	if (!JsonValue->TryGetObject(Object))
		return false;

	bool ParseSuccess = true;

	ParseSuccess &= TryGetJsonValue(*Object, TEXT("amount"), Amount);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("type"), Type);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("recipient_address"), RecipientAddress);

	return ParseSuccess;
}

}
