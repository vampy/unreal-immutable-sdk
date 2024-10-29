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

#include "APIERC721CollectionItem.h"

#include "ImmutablezkEVMAPIModule.h"
#include "APIHelpers.h"

#include "Templates/SharedPointer.h"

namespace ImmutablezkEVMAPI
{

inline FString ToString(const APIERC721CollectionItem::TypeEnum& Value)
{
	switch (Value)
	{
	case APIERC721CollectionItem::TypeEnum::ERC721COLLECTION:
		return TEXT("ERC721_COLLECTION");
	}

	UE_LOG(LogImmutablezkEVMAPI, Error, TEXT("Invalid APIERC721CollectionItem::TypeEnum Value (%d)"), (int)Value);
	return TEXT("");
}

FString APIERC721CollectionItem::EnumToString(const APIERC721CollectionItem::TypeEnum& EnumValue)
{
	return ToString(EnumValue);
}

inline bool FromString(const FString& EnumAsString, APIERC721CollectionItem::TypeEnum& Value)
{
	static TMap<FString, APIERC721CollectionItem::TypeEnum> StringToEnum = { 
		{ TEXT("ERC721_COLLECTION"), APIERC721CollectionItem::TypeEnum::ERC721COLLECTION }, };

	const auto Found = StringToEnum.Find(EnumAsString);
	if(Found)
		Value = *Found;

	return Found != nullptr;
}

bool APIERC721CollectionItem::EnumFromString(const FString& EnumAsString, APIERC721CollectionItem::TypeEnum& EnumValue)
{
	return FromString(EnumAsString, EnumValue);
}

inline void WriteJsonValue(JsonWriter& Writer, const APIERC721CollectionItem::TypeEnum& Value)
{
	WriteJsonValue(Writer, ToString(Value));
}

inline bool TryGetJsonValue(const TSharedPtr<FJsonValue>& JsonValue, APIERC721CollectionItem::TypeEnum& Value)
{
	FString TmpValue;
	if (JsonValue->TryGetString(TmpValue))
	{
		if(FromString(TmpValue, Value))
			return true;
	}
	return false;
}

void APIERC721CollectionItem::WriteJson(JsonWriter& Writer) const
{
	Writer->WriteObjectStart();
	Writer->WriteIdentifierPrefix(TEXT("type")); WriteJsonValue(Writer, Type);
	Writer->WriteIdentifierPrefix(TEXT("contract_address")); WriteJsonValue(Writer, ContractAddress);
	Writer->WriteIdentifierPrefix(TEXT("amount")); WriteJsonValue(Writer, Amount);
	Writer->WriteObjectEnd();
}

bool APIERC721CollectionItem::FromJson(const TSharedPtr<FJsonValue>& JsonValue)
{
	const TSharedPtr<FJsonObject>* Object;
	if (!JsonValue->TryGetObject(Object))
		return false;

	bool ParseSuccess = true;

	ParseSuccess &= TryGetJsonValue(*Object, TEXT("type"), Type);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("contract_address"), ContractAddress);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("amount"), Amount);

	return ParseSuccess;
}

}
