// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BBChatInput.h"
#include "Components/EditableTextBox.h"
#include "Player/BBPlayerController.h"

void UBBChatInput::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);		
	}	
}

void UBBChatInput::NativeDestruct()
{
	Super::NativeDestruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UBBChatInput::SetInputEnabled(bool bEnabled)
{
	bIsInputBlocked = !bEnabled;

	if (IsValid(EditableTextBox_ChatInput) == true)
	{
		// 위젯 자체를 비활성화하여 입력 차단
		EditableTextBox_ChatInput->SetIsEnabled(bEnabled);
	}
}

void UBBChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 입력이 차단된 상태면 무시
	if (bIsInputBlocked == true)
	{
		EditableTextBox_ChatInput->SetText(FText());
		return;
	}

	if (CommitMethod == ETextCommit::OnEnter)
	{
		APlayerController* OwningPlayerController = GetOwningPlayer();
		if (IsValid(OwningPlayerController) == true)
		{
			ABBPlayerController* OwningBBPlayerController = Cast<ABBPlayerController>(OwningPlayerController);
			if (IsValid(OwningBBPlayerController) == true)
			{
				OwningBBPlayerController->SetChatMessageString(Text.ToString());

				EditableTextBox_ChatInput->SetText(FText());
			}
		}
	}
}
