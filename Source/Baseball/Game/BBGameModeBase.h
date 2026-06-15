// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BBGameModeBase.generated.h"

class ABBPlayerController;

/**
 * 
 */
UCLASS()
class BASEBALL_API ABBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ABBGameModeBase();

	virtual void OnPostLogin(AController* NewPlayer) override;
	
	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	void PrintChatMessageString(ABBPlayerController* InChattingPlayerController, const FString& InChatMessageString);
	
	void IncreaseGuessCount(ABBPlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(ABBPlayerController* InChattingPlayerController, int InStrikeCount);
	
	void StartTurnTimer();
	void OnTurnTimeExpired();
	
	void AdvanceTurn(bool bGuessedThisTurn);

protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ABBPlayerController>> AllPlayerControllers;

	// 한 턴당 제한 시간
	UPROPERTY(EditDefaultsOnly, Category = "Turn")
	float TurnTimeLimit = 30.f;

	// 현재 남은 시간
	float CurrentRemainingTime = 0.f;

	// 타이머가 활성화되어 있는지
	bool bTurnTimerActive = false;

	// 현재 턴인 플레이어 인덱스
	int32 CurrentTurnIndex = 0;

	// 이번 턴에 숫자를 입력했는지 여부
	bool bHasGuessedThisTurn = false;
};