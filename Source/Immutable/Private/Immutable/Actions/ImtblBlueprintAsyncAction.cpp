// Fill out your copyright notice in the Description page of Project Settings.

#include "Immutable/Actions/ImtblBlueprintAsyncAction.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include "Immutable/ImmutableSubsystem.h"

UImmutableSubsystem* UImtblBlueprintAsyncAction::GetSubsystem() const
{
	if (!IsValid(RetainedWorldContextObject))
	{
		return nullptr;
	}

	UWorld* World = RetainedWorldContextObject->GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UImmutableSubsystem>() : nullptr;
}
