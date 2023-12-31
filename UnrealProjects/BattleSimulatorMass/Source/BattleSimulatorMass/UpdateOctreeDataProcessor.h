// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "UpdateOctreeDataProcessor.generated.h"

// Processor that updates the octree based on where all entities are

UCLASS()
class BATTLESIMULATORMASS_API UUpdateOctreeDataProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UUpdateOctreeDataProcessor();

protected:
	virtual void Initialize(UObject& Owner)override;
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)override;

private:
	FMassEntityQuery EntityQuery;

	class UTargetAcquisitionOctreeSubsystem* TargetAcquisitionSubsystem;
};
