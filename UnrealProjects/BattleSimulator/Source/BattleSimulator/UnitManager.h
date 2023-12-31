// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitManager.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BATTLESIMULATOR_API UUnitManager : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UUnitManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TArray<AActor*>& GetAllEnemyUnits(int TeamId);

	void UnRegisterUnit(AActor* Unit);

	int GetNextTeamId()const;

	void SpawnUnits();

	void SetSpawnCount(int Count);
	int GetSpawnCount()const;

	void SetMinBounds(const FVector& Bounds);
	void SetMaxBounds(const FVector& Bounds);
	const FVector& GetMinBounds()const;
	const FVector& GetMaxBounds()const;

private:
	void SpawnUnit(const FTransform& SpawnTransform);

	TArray<TArray<AActor*>> Units;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int SpawnCount = 100;

	UPROPERTY(EditAnywhere)
		FVector MinBounds{-5000,-5000, 0};

	UPROPERTY(EditAnywhere)
		FVector MaxBounds{5000, 5000, 0};

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> UnitTemplate;

	int NextTeamId = 0;

};
