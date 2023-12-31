// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackProcessor.h"
#include "UnitFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "UnitTags.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "TargetAcquisitionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "BattleSimGameMode.h"

UAttackProcessor::UAttackProcessor()
	:EntityQuery{ *this }
{
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionFlags = int32(EProcessorExecutionFlags::All);
}

void UAttackProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	GameMode = Cast<ABattleSimGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UMovementProcessor: GameMode not found."));
	}
}

void UAttackProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FUnitAnimStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FUnitAttackFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FUnitTargetAcquisitionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FUnitAttackParameters>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FDeadTag>(EMassFragmentPresence::None);
	EntityQuery.AddTagRequirement<FDyingTag>(EMassFragmentPresence::None);
}

void UAttackProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
#ifdef ENABLE_MULTITHREADING
	if (!GameMode->HasStartedSimulation()) return;

	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([&](FMassExecutionContext& Context)
		{
			const TArrayView<FUnitAnimStateFragment> AnimStateList = Context.GetMutableFragmentView<FUnitAnimStateFragment>();
			const TArrayView<FUnitAttackFragment> AttackList = Context.GetMutableFragmentView<FUnitAttackFragment>();
			const TConstArrayView<FUnitTargetAcquisitionFragment> TargetAcquisitionList = Context.GetFragmentView<FUnitTargetAcquisitionFragment>();
			const FUnitAttackParameters& AttackParams = Context.GetConstSharedFragment<FUnitAttackParameters>();
			const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

			ParallelFor(Context.GetNumEntities(), [&](int32 EntityIndex)
			{
				const FMassEntityHandle& TargetEntity = TargetAcquisitionList[EntityIndex].CurrentTarget;

				if (EntityManager.IsEntityValid(TargetEntity) && TargetAcquisitionList[EntityIndex].ClosestTargetDistanceSqr <= FMath::Square(AttackParams.Range))
				{
					//Timer
					if (AttackList[EntityIndex].AttackDelayTimer > 0)
					{
						AttackList[EntityIndex].AttackDelayTimer -= WorldDeltaTime;

						if (AttackList[EntityIndex].AttackDelayTimer <= AttackParams.AnimationAttackDelay)
						{
							if (AnimStateList[EntityIndex].UnitAnimState != EUnitAnimState::Dead)
							{
								AnimStateList[EntityIndex].UnitAnimState = EUnitAnimState::Attacking;
							}
						}

						if (AttackList[EntityIndex].AttackDelayTimer > 0) return;
					}


					// ATTACK

					//Reset timer
					AttackList[EntityIndex].AttackDelayTimer += AttackParams.AttackDelay;

					//Get health fragment
					FMassEntityView EntityView = FMassEntityView(EntityManager, TargetEntity);
					FUnitHealthFragment& TargetEntityHealth = EntityView.GetFragmentData<FUnitHealthFragment>();

					//Do damage
					TargetEntityHealth.CurrentHealth -= AttackParams.Damage;

					//Check if dead
					if (TargetEntityHealth.CurrentHealth <= 0)
					{

						Mutex.Lock();
						Context.Defer().AddTag<FDyingTag>(TargetEntity);
						Mutex.Unlock();
					}

					AnimStateList[EntityIndex].UnitAnimState = EUnitAnimState::Idle;
				}
				else
				{
					//Count down timer until anim delay, so we are ready for the next attack
					if (AttackList[EntityIndex].AttackDelayTimer > AttackParams.AnimationAttackDelay)
					{
						AttackList[EntityIndex].AttackDelayTimer -= WorldDeltaTime;
					}
					else
					{
						AttackList[EntityIndex].AttackDelayTimer = AttackParams.AnimationAttackDelay;
						if (AnimStateList[EntityIndex].UnitAnimState == EUnitAnimState::Attacking)
						{
							AnimStateList[EntityIndex].UnitAnimState = EUnitAnimState::Idle;
						}
					}					
				}
			});
		}));
#else
		if (!GameMode->HasStartedSimulation()) return;

		EntityQuery.ForEachEntityChunk(EntityManager, Context, ([&](FMassExecutionContext& Context)
			{
				const TArrayView<FUnitAnimStateFragment> AnimStateList = Context.GetMutableFragmentView<FUnitAnimStateFragment>();
				const TArrayView<FUnitAttackFragment> AttackList = Context.GetMutableFragmentView<FUnitAttackFragment>();
				const TConstArrayView<FUnitTargetAcquisitionFragment> TargetAcquisitionList = Context.GetFragmentView<FUnitTargetAcquisitionFragment>();
				const FUnitAttackParameters& AttackParams = Context.GetConstSharedFragment<FUnitAttackParameters>();
				const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

				for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
				{
					const FMassEntityHandle& TargetEntity = TargetAcquisitionList[EntityIndex].CurrentTarget;

					if (EntityManager.IsEntityValid(TargetEntity) && TargetAcquisitionList[EntityIndex].ClosestTargetDistanceSqr <= FMath::Square(AttackParams.Range))
					{
						//Timer
						if (AttackList[EntityIndex].AttackDelayTimer > 0)
						{
							AttackList[EntityIndex].AttackDelayTimer -= WorldDeltaTime;

							if (AttackList[EntityIndex].AttackDelayTimer <= AttackParams.AnimationAttackDelay)
							{
								if (AnimStateList[EntityIndex].UnitAnimState != EUnitAnimState::Dead)
								{
									AnimStateList[EntityIndex].UnitAnimState = EUnitAnimState::Attacking;
								}
							}

							if (AttackList[EntityIndex].AttackDelayTimer > 0) continue;
						}


						// ATTACK

						//Increase timer
						AttackList[EntityIndex].AttackDelayTimer += AttackParams.AttackDelay;

						//Get health fragment
						FMassEntityView EntityView = FMassEntityView(EntityManager, TargetEntity);
						FUnitHealthFragment& TargetEntityHealth = EntityView.GetFragmentData<FUnitHealthFragment>();

						//Do damage
						TargetEntityHealth.CurrentHealth -= AttackParams.Damage;

						//Check if dead
						if (TargetEntityHealth.CurrentHealth <= 0)
						{
							Context.Defer().AddTag<FDyingTag>(TargetEntity);
						}

						AnimStateList[EntityIndex].UnitAnimState = EUnitAnimState::Idle;
					}
					else
					{
						//Count down timer until anim delay, so we are ready for the next attack
						if (AttackList[EntityIndex].AttackDelayTimer > AttackParams.AnimationAttackDelay)
						{
							AttackList[EntityIndex].AttackDelayTimer -= WorldDeltaTime;
						}
						else
						{
							AttackList[EntityIndex].AttackDelayTimer = AttackParams.AnimationAttackDelay;
						}

						if (AnimStateList[EntityIndex].UnitAnimState == EUnitAnimState::Attacking)
						{
							AnimStateList[EntityIndex].UnitAnimState = EUnitAnimState::Idle;
						}
					}
				}
		}));
#endif // ENABLE_MULTITHREADING	
}
