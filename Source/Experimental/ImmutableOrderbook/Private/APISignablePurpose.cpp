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

#include "APISignablePurpose.h"

#include "ImmutableOrderbookModule.h"
#include "APIHelpers.h"

#include "Templates/SharedPointer.h"

namespace ImmutableOrderbook
{

inline FString ToString(const APISignablePurpose::Values& Value)
{
	switch (Value)
	{
	case APISignablePurpose::Values::CreateListing:
		return TEXT("CREATE_LISTING");
	case APISignablePurpose::Values::OffChainCancellation:
		return TEXT("OFF_CHAIN_CANCELLATION");
	}

	UE_LOG(LogImmutableOrderbook, Error, TEXT("Invalid APISignablePurpose::Values Value (%d)"), (int)Value);
	return TEXT("");
}

FString APISignablePurpose::EnumToString(const APISignablePurpose::Values& EnumValue)
{
	return ToString(EnumValue);
}

inline bool FromString(const FString& EnumAsString, APISignablePurpose::Values& Value)
{
	static TMap<FString, APISignablePurpose::Values> StringToEnum = { 
		{ TEXT("CREATE_LISTING"), APISignablePurpose::Values::CreateListing },
		{ TEXT("OFF_CHAIN_CANCELLATION"), APISignablePurpose::Values::OffChainCancellation }, };

	const auto Found = StringToEnum.Find(EnumAsString);
	if(Found)
		Value = *Found;

	return Found != nullptr;
}

bool APISignablePurpose::EnumFromString(const FString& EnumAsString, APISignablePurpose::Values& EnumValue)
{
	return FromString(EnumAsString, EnumValue);
}

inline void WriteJsonValue(JsonWriter& Writer, const APISignablePurpose::Values& Value)
{
	WriteJsonValue(Writer, ToString(Value));
}

inline bool TryGetJsonValue(const TSharedPtr<FJsonValue>& JsonValue, APISignablePurpose::Values& Value)
{
	FString TmpValue;
	if (JsonValue->TryGetString(TmpValue))
	{
		if(FromString(TmpValue, Value))
			return true;
	}
	return false;
}

void APISignablePurpose::WriteJson(JsonWriter& Writer) const
{
	WriteJsonValue(Writer, Value);
}

bool APISignablePurpose::FromJson(const TSharedPtr<FJsonValue>& JsonValue)
{
	return TryGetJsonValue(JsonValue, Value);
}

}
