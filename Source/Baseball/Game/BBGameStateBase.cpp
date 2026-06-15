// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BBGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BBPlayerController.h"

void ABBGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			ABBPlayerController* CXPC = Cast<ABBPlayerController>(PC);
			if (IsValid(CXPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				CXPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}
