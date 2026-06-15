// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BBPlayerController.generated.h"

class UBBChatInput;
class UUserWidget;

/**
 * 
 */
UCLASS()
class BASEBALL_API ABBPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABBPlayerController();
	
	virtual void BeginPlay() override;
	
	void SetChatMessageString(const FString& InChatMessageString);
	
	void PrintChatMessageString(const FString& InChatMessageString);
	
	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

	// 입력 차단 / 해제
	UFUNCTION(Client, Reliable)
	void ClientRPCSetInputBlocked(bool bBlocked);

	void SetInputBlocked(bool bBlocked);

	// 타이머 시간 업데이트 
	UFUNCTION(Client, Unreliable)
	void ClientRPCUpdateTimer(float InRemainingTime);

	// 알림 텍스트 업데이트 
	UFUNCTION(Client, Reliable)
	void ClientRPCSetNotificationText(const FString& InNotificationString);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBBChatInput> ChatInputWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UBBChatInput> ChatInputWidgetInstance;
	
	FString ChatMessageString;
	
	// 알림 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

	// 타이머 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> TimerWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> TimerWidgetInstance;
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_NotificationText, BlueprintReadOnly)
	FText NotificationText;

	UFUNCTION()
	void OnRep_NotificationText();
	
	UPROPERTY(BlueprintReadOnly)
	float RemainingTime = 30.f;
};
