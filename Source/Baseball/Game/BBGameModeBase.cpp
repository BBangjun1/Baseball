// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BBGameModeBase.h"
#include "BBGameStateBase.h"
#include "Player/BBPlayerController.h"
#include "EngineUtils.h"
#include "Player/BBPlayerState.h"

ABBGameModeBase::ABBGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABBGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABBPlayerController* BBPlayerController = Cast<ABBPlayerController>(NewPlayer);
	if (IsValid(BBPlayerController) == true)
	{
		BBPlayerController->ClientRPCSetNotificationText(TEXT("Connected to the game server."));
		
		AllPlayerControllers.Add(BBPlayerController);

		ABBPlayerState* BBPS = BBPlayerController->GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			BBPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ABBGameStateBase* BBGameStateBase = GetGameState<ABBGameStateBase>();
		if (IsValid(BBGameStateBase) == true)
		{
			BBGameStateBase->MulticastRPCBroadcastLoginMessage(BBPS->PlayerNameString);
		}
	}
}

FString ABBGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });
	
	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ABBGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}
			
			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		if (UniqueDigits.Num() != 3)
		{
			break;
		}

		bCanPlay = true;
		
	} while (false);	

	return bCanPlay;
}

FString ABBGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else 
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;				
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ABBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);

	// 게임 시작 시 타이머 시작
	StartTurnTimer();
}

void ABBGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bTurnTimerActive == false)
	{
		return;
	}

	CurrentRemainingTime -= DeltaSeconds;

	// 각 클라이언트에 RPC로 직접 타이머 전송
	static float LastSentTime = -1.f;
	float FlooredTime = FMath::FloorToFloat(CurrentRemainingTime);
	if (FlooredTime != LastSentTime)
	{
		LastSentTime = FlooredTime;
		for (const auto& BBPlayerController : AllPlayerControllers)
		{
			if (IsValid(BBPlayerController) == true)
			{
				BBPlayerController->ClientRPCUpdateTimer(FlooredTime);
			}
		}
	}

	// 시간이 0 이하가 되면 턴 종료 처리
	if (CurrentRemainingTime <= 0.f)
	{
		CurrentRemainingTime = 0.f;
		bTurnTimerActive = false;
		OnTurnTimeExpired();
	}
}

void ABBGameModeBase::StartTurnTimer()
{
	CurrentRemainingTime = TurnTimeLimit;
	bTurnTimerActive = true;
	bHasGuessedThisTurn = false;

	// 현재 턴 플레이어만 입력 허용, 나머지는 차단
	for (int32 i = 0; i < AllPlayerControllers.Num(); ++i)
	{
		ABBPlayerController* BBPlayerController = AllPlayerControllers[i];
		if (IsValid(BBPlayerController) == true)
		{
			bool bIsMyTurn = (i == CurrentTurnIndex);
			BBPlayerController->ClientRPCSetInputBlocked(!bIsMyTurn);

			// 현재 턴 플레이어 이름 공지
			if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex))
			{
				ABBPlayerState* TurnPS = AllPlayerControllers[CurrentTurnIndex]->GetPlayerState<ABBPlayerState>();
				FString TurnPlayerName = IsValid(TurnPS) ? TurnPS->PlayerNameString : TEXT("Unknown");
				FString TurnMsg = FString::Printf(TEXT("[System] %s's turn!"), *TurnPlayerName);
				BBPlayerController->ClientRPCPrintChatMessageString(TurnMsg);
			}
		}
	}
}

void ABBGameModeBase::OnTurnTimeExpired()
{
	// 시간 초과 시 해당 플레이어가 숫자를 입력하지 않은 것으로 처리
	for (const auto& BBPlayerController : AllPlayerControllers)
	{
		if (IsValid(BBPlayerController) == true)
		{
			BBPlayerController->ClientRPCPrintChatMessageString(TEXT("[System] Time is up! Turn skipped."));
		}
	}

	AdvanceTurn(false);
}

void ABBGameModeBase::PrintChatMessageString(ABBPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	// 자기 턴이 아닌 플레이어의 숫자 입력 차단
	int32 ChattingPlayerIndex = AllPlayerControllers.IndexOfByKey(InChattingPlayerController);
	if (ChattingPlayerIndex != CurrentTurnIndex)
	{
		for (TActorIterator<ABBPlayerController> It(GetWorld()); It; ++It)
		{
			ABBPlayerController* BBPlayerController = *It;
			if (IsValid(BBPlayerController) == true)
			{
				BBPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
		return;
	}

	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true)
	{
		IncreaseGuessCount(InChattingPlayerController);
		
		ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();
		FString UpdatedPlayerInfo = IsValid(BBPS) ? BBPS->GetPlayerInfoString() : TEXT("Unknown");

		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		
		for (TActorIterator<ABBPlayerController> It(GetWorld()); It; ++It)
		{
			ABBPlayerController* BBPlayerController = *It;
			if (IsValid(BBPlayerController) == true)
			{
				FString CombinedMessageString = UpdatedPlayerInfo + TEXT(": ") + GuessNumberString + TEXT(" -> ") + JudgeResultString;
				BBPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}

		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));

		// 승리 시 리셋하고 종료
		if (StrikeCount == 3)
		{
			JudgeGame(InChattingPlayerController, StrikeCount);
			return;
		}

		JudgeGame(InChattingPlayerController, StrikeCount);

		// 숫자 입력 성공 시 활동 체크 후 턴 전환
		bHasGuessedThisTurn = true;
		AdvanceTurn(true);
	}
	else
	{
		for (TActorIterator<ABBPlayerController> It(GetWorld()); It; ++It)
		{
			ABBPlayerController* BBPlayerController = *It;
			if (IsValid(BBPlayerController) == true)
			{
				BBPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}

void ABBGameModeBase::IncreaseGuessCount(ABBPlayerController* InChattingPlayerController)
{
	ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();
	if (IsValid(BBPS) == true)
	{
		BBPS->CurrentGuessCount++;
	}
}

void ABBGameModeBase::AdvanceTurn(bool bGuessedThisTurn)
{
	// 숫자를 입력하지 않은 경우 시도 횟수 1회 차감
	if (bGuessedThisTurn == false)
	{
		if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex))
		{
			ABBPlayerState* BBPS = AllPlayerControllers[CurrentTurnIndex]->GetPlayerState<ABBPlayerState>();
			if (IsValid(BBPS) == true)
			{
				BBPS->CurrentGuessCount++;
			}
		}
	}

	// 무승부 체크 
	bool bIsDraw = true;
	for (const auto& BBPlayerController : AllPlayerControllers)
	{
		ABBPlayerState* BBPS = BBPlayerController->GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			if (BBPS->CurrentGuessCount < BBPS->MaxGuessCount)
			{
				bIsDraw = false;
				break;
			}
		}
	}

	if (bIsDraw == true)
	{
		for (const auto& BBPlayerController : AllPlayerControllers)
		{
			BBPlayerController->ClientRPCSetNotificationText(TEXT("Draw..."));
		}
		ResetGame();
		UE_LOG(LogTemp, Error, TEXT("Draw! New secret: %s"), *SecretNumberString);
		return;
	}

	if (AllPlayerControllers.Num() > 0)
	{
		CurrentTurnIndex = (CurrentTurnIndex + 1) % AllPlayerControllers.Num();
	}

	StartTurnTimer();
}


void ABBGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();
	CurrentTurnIndex = 0;
	bHasGuessedThisTurn = false;

	for (const auto& BBPlayerController : AllPlayerControllers)
	{
		ABBPlayerState* BBPS = BBPlayerController->GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			BBPS->CurrentGuessCount = 0;
		}
	}

	StartTurnTimer();
}

void ABBGameModeBase::JudgeGame(ABBPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();
		for (const auto& BBPlayerController : AllPlayerControllers)
		{
			if (IsValid(BBPS) == true)
			{
				FString CombinedMessageString = BBPS->PlayerNameString + TEXT(" has won the game.");
				BBPlayerController->ClientRPCSetNotificationText(CombinedMessageString);
			}
		}

		ResetGame();
		UE_LOG(LogTemp, Error, TEXT("Win! New secret: %s"), *SecretNumberString);
	}
}