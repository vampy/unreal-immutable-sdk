// Fill out your copyright notice in the Description page of Project Settings.

#include "Immutable/Actions/ImtblPassportGetLinkedAddressesAsyncAction.h"

#include "Immutable/ImmutablePassport.h"
#include "Immutable/ImmutableSubsystem.h"
#include "Immutable/Misc/ImtblLogging.h"

UImtblPassportGetLinkedAddressesAsyncAction* UImtblPassportGetLinkedAddressesAsyncAction::GetLinkedAddresses(UObject* WorldContextObject)
{
	UImtblPassportGetLinkedAddressesAsyncAction* Node = NewObject<UImtblPassportGetLinkedAddressesAsyncAction>();
	
	Node->WorldContextObject = WorldContextObject;
	
	return Node;
}

void UImtblPassportGetLinkedAddressesAsyncAction::Activate()
{
	UImmutableSubsystem* Subsystem = GetSubsystem();
	if (!Subsystem)
	{
		FString Err = "GetLinkedAddresses failed due to missing world or world context object.";
		
		IMTBL_WARN("%s", *Err)
		Failed.Broadcast(Err, TArray<FString>());
		
		return;
	}

	Subsystem->WhenReady(this, &UImtblPassportGetLinkedAddressesAsyncAction::DoGetLinkedAddresses);
}

void UImtblPassportGetLinkedAddressesAsyncAction::DoGetLinkedAddresses(TWeakObjectPtr<UImtblJSConnector> JSConnector)
{
	auto Passport = GetSubsystem()->GetPassport();

	if (Passport.IsValid())
	{
		Passport->GetLinkedAddresses(UImmutablePassport::FImtblPassportResponseDelegate::CreateUObject(this, &UImtblPassportGetLinkedAddressesAsyncAction::OnGetLinkedAddressesResponse));
	}
}

void UImtblPassportGetLinkedAddressesAsyncAction::OnGetLinkedAddressesResponse(FImmutablePassportResult Result)
{
	if (Result.Success)
	{
		Success.Broadcast(TEXT(""), UImmutablePassport::GetResponseResultAsStringArray(Result.Response));
	}
	else
	{
		Failed.Broadcast(Result.Error, TArray<FString>());
	}
}
