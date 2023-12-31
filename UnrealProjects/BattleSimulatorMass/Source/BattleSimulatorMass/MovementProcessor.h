// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MovementProcessor.generated.h"

// Processor that handles whether a unit should move or not, and sets some variables accordingly 

UCLASS()
class BATTLESIMULATORMASS_API UMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMovementProcessor();

protected:
	virtual void Initialize(UObject& Owner)override;
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)override;

private:
	FMassEntityQuery EntityQuery;

	class ABattleSimGameMode* GameMode;
};
