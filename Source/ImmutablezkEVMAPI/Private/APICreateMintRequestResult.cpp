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

#include "APICreateMintRequestResult.h"

#include "ImmutablezkEVMAPIModule.h"
#include "APIHelpers.h"

#include "Templates/SharedPointer.h"

namespace ImmutablezkEVMAPI
{

void APICreateMintRequestResult::WriteJson(JsonWriter& Writer) const
{
	Writer->WriteObjectStart();
	Writer->WriteIdentifierPrefix(TEXT("imx_mint_requests_limit")); WriteJsonValue(Writer, ImxMintRequestsLimit);
	Writer->WriteIdentifierPrefix(TEXT("imx_mint_requests_limit_reset")); WriteJsonValue(Writer, ImxMintRequestsLimitReset);
	Writer->WriteIdentifierPrefix(TEXT("imx_remaining_mint_requests")); WriteJsonValue(Writer, ImxRemainingMintRequests);
	Writer->WriteIdentifierPrefix(TEXT("imx_mint_requests_retry_after")); WriteJsonValue(Writer, ImxMintRequestsRetryAfter);
	Writer->WriteObjectEnd();
}

bool APICreateMintRequestResult::FromJson(const TSharedPtr<FJsonValue>& JsonValue)
{
	const TSharedPtr<FJsonObject>* Object;
	if (!JsonValue->TryGetObject(Object))
		return false;

	bool ParseSuccess = true;

	ParseSuccess &= TryGetJsonValue(*Object, TEXT("imx_mint_requests_limit"), ImxMintRequestsLimit);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("imx_mint_requests_limit_reset"), ImxMintRequestsLimitReset);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("imx_remaining_mint_requests"), ImxRemainingMintRequests);
	ParseSuccess &= TryGetJsonValue(*Object, TEXT("imx_mint_requests_retry_after"), ImxMintRequestsRetryAfter);

	return ParseSuccess;
}

}
