// CXPawn.cpp


#include "BBPawn.h"
#include "Baseball.h"


void ABBPawn::BeginPlay()
{
	Super::BeginPlay();

	FString NetRoleString = BaseballFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("BBPawn::BeginPlay() %s [%s]"), *BaseballFunctionLibrary::GetNetModeString(this), *NetRoleString);
	BaseballFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}

void ABBPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	FString NetRoleString = BaseballFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("BBPawn::PossessedBy() %s [%s]"), *BaseballFunctionLibrary::GetNetModeString(this), *NetRoleString);
	BaseballFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}
