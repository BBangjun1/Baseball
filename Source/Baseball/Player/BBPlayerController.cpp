// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BBPlayerController.h"
#include "Baseball.h"
#include "EngineUtils.h"
#include "UI/BBChatInput.h"
#include "Kismet/GameplayStatics.h"
#include "Game/BBGameModeBase.h"
#include "Player/BBPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Components/TextBlock.h"

ABBPlayerController::ABBPlayerController()
{
	bReplicates = true;
}

void ABBPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
	{
		return;
	}
	
	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UBBChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}
	
	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(TimerWidgetClass) == true)
	{
		TimerWidgetInstance = CreateWidget<UUserWidget>(this, TimerWidgetClass);
		if (IsValid(TimerWidgetInstance) == true)
		{
			TimerWidgetInstance->AddToViewport();
		}
	}
}

void ABBPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;
	
	if (IsLocalController() == true)
	{
		ABBPlayerState* BBPS = GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			FString CombinedMessageString = BBPS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;
			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void ABBPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	BaseballFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void ABBPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ABBPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ABBGameModeBase* BBGM = Cast<ABBGameModeBase>(GM);
		if (IsValid(BBGM) == true)
		{
			BBGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void ABBPlayerController::ClientRPCSetInputBlocked_Implementation(bool bBlocked)
{
	SetInputBlocked(bBlocked);
}

void ABBPlayerController::SetInputBlocked(bool bBlocked)
{
	if (IsValid(ChatInputWidgetInstance) == false)
	{
		return;
	}

	ChatInputWidgetInstance->SetInputEnabled(!bBlocked);

	if (bBlocked)
	{
		BaseballFunctionLibrary::MyPrintString(this, TEXT("[System] Input blocked - Time is up!"), 5.f, FColor::Red);
	}
	else
	{
		BaseballFunctionLibrary::MyPrintString(this, TEXT("[System] Input enabled - Enter your guess!"), 3.f, FColor::Green);
	}
}

void ABBPlayerController::ClientRPCUpdateTimer_Implementation(float InRemainingTime)
{
	RemainingTime = InRemainingTime;

	if (IsValid(TimerWidgetInstance) == false)
	{
		return;
	}

	UTextBlock* TimerTextBlock = Cast<UTextBlock>(TimerWidgetInstance->GetWidgetFromName(TEXT("TimerText")));
	if (IsValid(TimerTextBlock) == true)
	{
		FString TimerString = FString::FromInt(FMath::CeilToInt(InRemainingTime)) + TEXT("초");
		TimerTextBlock->SetText(FText::FromString(TimerString));
	}
}

void ABBPlayerController::ClientRPCSetNotificationText_Implementation(const FString& InNotificationString)
{
	NotificationText = FText::FromString(InNotificationString);
	OnRep_NotificationText();
}

void ABBPlayerController::OnRep_NotificationText()
{
	// 알림 위젯에 텍스트 출력 
	BaseballFunctionLibrary::MyPrintString(this, NotificationText.ToString(), 5.f, FColor::Yellow);
}

void ABBPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}