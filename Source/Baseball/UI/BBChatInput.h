// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "BBChatInput.generated.h"

class UEditableTextBox;

/**
 * 
 */
UCLASS()
class BASEBALL_API UBBChatInput : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	// 입력 활성/비활성화
	void SetInputEnabled(bool bEnabled);

protected:
	UFUNCTION()
	void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;

private:
	// 현재 입력이 차단되어 있는지
	bool bIsInputBlocked = false;
};
