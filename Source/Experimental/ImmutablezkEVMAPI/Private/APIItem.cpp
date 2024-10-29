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

#include "APIItem.h"

#include "ImmutablezkEVMAPIModule.h"
#include "APIHelpers.h"

#include "Templates/SharedPointer.h"

namespace ImmutablezkEVMAPI
{

inline FString ToString(const APIItem::TypeEnum& Value)
{
	switch (Value)
	{
	case APIItem::TypeEnum::Native:
		return TEXT("NATIVE");
	case APIItem::TypeEnum::ERC20:
		return TEXT("ERC20");
	case APIItem::TypeEnum::ERC721:
		return TEXT("ERC721");
	case APIItem::TypeEnum::ERC1155:
		return TEXT("ERC1155");
	case APIItem::TypeEnum::ERC721COLLECTION:
		return TEXT("ERC721_COLLECTION");
	case APIItem::TypeEnum::ERC1155COLLECTION:
		return TEXT("ERC1155_COLLECTION");
	}

	UE_LOG(LogImmutablezkEVMAPI, Error, TEXT("Invalid APIItem::TypeEnum Value (%d)"), (int)Value);
	return TEXT("");
}

FString APIItem::EnumToString(const APIItem::TypeEnum& EnumValue)
{
	return ToString(EnumValue);
}

inline bool FromString(const FString& EnumAsString, APIItem::TypeEnum& Value)
{
	static TMap<FString, APIItem::TypeEnum> StringToEnum = { 
		{ TEXT("NATIVE"), APIItem::TypeEnum::Native },
		{ TEXT("ERC20"), APIItem::TypeEnum::ERC20 },
		{ TEXT("ERC721"), APIItem::TypeEnum::ERC721 },
		{ TEXT("ERC1155"), APIItem::TypeEnum::ERC1155 },
		{ TEXT("ERC721_COLLECTION"), APIItem::TypeEnum::ERC721COLLECTION },
		{ TEXT("ERC1155_COLLECTION"), APIItem::TypeEnum::ERC1155COLLECTION }, };

	const auto Found = StringToEnum.Find(EnumAsString);
	if(Found)
		Value = *Found;

	return Found != nullptr;
}

bool APIItem::EnumFromString(const FString& EnumAsString, APIItem::TypeEnum& EnumValue)
{
	return FromString(EnumAsString, EnumValue);
}

inline void WriteJsonValue(JsonWriter& Writer, const APIItem::TypeEnum& Value)
{
	WriteJsonValue(Writer, ToString(Value));
}

inline bool TryGetJsonValue(const TSharedPtr<FJsonValue>& JsonValue, APIItem::TypeEnum& Value)
{
	FString TmpValue;
	if (JsonValue->TryGetString(TmpValue))
	{
		if(FromString(TmpValue, Value))
			return true;
	}
	return false;
}

void APIItem::WriteJson(JsonWriter& Writer) const
{
	Writer->WriteObjectStart();
	Writer->WriteIdentifierPrefix(TEXT("type")); WriteJsonValue(Writer, Type);
	Writer->WriteIdentifierPrefix(TEXT("amount")); WriteJsonValue(Writer, Amount);
	Writer->WriteIdentifierPrefix(TEXT("contract_address")); WriteJsonValue(Writer, ContractAddress);
	Writer->WriteIdentifierPrefix(TEXT("token_id")); WriteJsonValue(Writer, TokenId);
	Writer->WriteObjectEnd();
}

bool APIItem::FromJson(const TSharedPtr<FJsonValue>& JsonValue)
{
	const TSharedPtr<FJsonObject>* Object;
	if (!JsonValue->TryGetObject(Object))
		return false;

	bool ParseSuccess = true;

	ParseSuccess &= TryGetJsonValue(*Object, TEXT("type"), Type);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("amount"), Amount);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("contract_address"), ContractAddress);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("token_id"), TokenId);

	return ParseSuccess;
}

}
