// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitTraits.h"
#include "UnitFragments.h"
#include "UnitTags.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityTemplate.h"
#include "UnitTranslators.h"
#include "MassEntityView.h"
#include "Translators/MassSceneComponentLocationTranslator.h"

// ArmyId
void UArmyIdTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FArmyIdFragment>();

#ifdef ENABLE_SPATIAL
	BuildContext.AddFragment<FUnitOctreeDataFragment>();
#endif // ENABLE_SPATIAL
}

// Attack
void UAttackTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct AttackParamsFragment = EntityManager.GetOrCreateConstSharedFragment(AttackParameters);
	BuildContext.AddConstSharedFragment(AttackParamsFragment);

	BuildContext.AddFragment<FUnitAttackFragment>();
}

// DeathHandling
void UDeathHandlingTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct DeathParamsFragment = EntityManager.GetOrCreateConstSharedFragment(DeathParameters);
	BuildContext.AddConstSharedFragment(DeathParamsFragment);

	FUnitDeathFragment& DeathTemplate = BuildContext.AddFragment_GetRef<FUnitDeathFragment>();
	DeathTemplate.DeathDelayTimer = DeathParameters.AnimationDeathDelay;
}

// Health
void UHealthTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FUnitHealthFragment& HealthTemplate = BuildContext.AddFragment_GetRef<FUnitHealthFragment>();
	HealthTemplate.CurrentHealth = Health;
}

// Move
void UMoveTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct MoveParamsFragment = EntityManager.GetOrCreateConstSharedFragment(MoveParameters);
	BuildContext.AddConstSharedFragment(MoveParamsFragment);
}

// TargetAcquisition
void UTargetAcquisitionTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment_GetRef<FUnitTargetAcquisitionFragment>();
}

// Animation
void UUnitAnimationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FUnitAnimStateFragment>();
}

// TransformSync
void UUnitTransformSyncTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMassSceneComponentWrapperFragment>();

	BuildContext.AddTag<FSyncTransformTag>();

	BuildContext.GetMutableObjectFragmentInitializers().Add([=](UObject& Owner, FMassEntityView& EntityView, const EMassTranslationDirection CurrentDirection)
		{
			AActor* AsActor = Cast<AActor>(&Owner);
			if (AsActor && AsActor->GetRootComponent())
			{
				USceneComponent* Component = AsActor->GetRootComponent();
				FMassSceneComponentWrapperFragment& ComponentFragment = EntityView.GetFragmentData<FMassSceneComponentWrapperFragment>();
				ComponentFragment.Component = Component;
			}
		});

	BuildContext.AddTranslator<UUnitTransformTranslator>();
}

// Visualization
void UUnitVisualizationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct VisualizationParamsFragment = EntityManager.GetOrCreateConstSharedFragment(VisualizationParameters);
	BuildContext.AddConstSharedFragment(VisualizationParamsFragment);
}
