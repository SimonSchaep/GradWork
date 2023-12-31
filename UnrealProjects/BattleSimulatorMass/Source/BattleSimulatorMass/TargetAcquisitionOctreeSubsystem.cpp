// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetAcquisitionOctreeSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassCommonTypes.h"
#include "MassEntityManager.h"
#include "MassEntityView.h"
#include "UnitFragments.h"
#include "DrawDebugHelpers.h"


UTargetAcquisitionOctreeSubsystem::UTargetAcquisitionOctreeSubsystem()
{

}


TStatId UTargetAcquisitionOctreeSubsystem::GetStatId() const
{
	return TStatId();
}

void UTargetAcquisitionOctreeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UMassSimulationSubsystem>();

	EntityManager = UE::Mass::Utils::GetEntityManager(GetWorld());	
}

void UTargetAcquisitionOctreeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Disabling tick doesn't seem to work, so we use this return instead
#ifndef ENABLE_SPATIAL
	return;
#endif // ENABLE_SPATIAL


	//Print unit counts
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("Unit Count:")), false);
		for (int32 i{}; i < UnitCounts.Num(); ++i)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("Army %i: %i"), i, UnitCounts[i]), false);
		}			
	}

	//Debug drawing code copied from: https://dev.epicgames.com/community/snippets/2eB/toctree2
	if (bDrawDebug)
	{
		for (const FUnitOctree& Octree : Octrees)
		{
			Octree.FindNodesWithPredicate(
				[this](FUnitOctree::FNodeIndex /*ParentNodeIndex*/, FUnitOctree::FNodeIndex /*NodeIndex*/, const FBoxCenterAndExtent& /*NodeBounds*/)
				{
					return true;
				},
				[this, &Octree](FUnitOctree::FNodeIndex /*ParentNodeIndex*/, FUnitOctree::FNodeIndex NodeIndex, const FBoxCenterAndExtent& NodeBounds)
				{
					FVector MaxExtent = NodeBounds.Extent;
					FVector Center = NodeBounds.Center;

					DrawDebugBox(GetWorld(), Center, MaxExtent, FColor().Blue, false, 0.0f);
					DrawDebugSphere(GetWorld(), Center + MaxExtent, 4.0f, 12, FColor().Green, false, 0.0f);
					DrawDebugSphere(GetWorld(), Center - MaxExtent, 4.0f, 12, FColor().Red, false, 0.0f);

					TArrayView<const FUnitOctreeElement> Elements = Octree.GetElementsForNode(NodeIndex);

					for (int i = 0; i < Elements.Num(); i++)
					{
						// Draw debug boxes around elements
						float Max = Elements[i].Bounds.BoxExtent.GetMax();
						MaxExtent = FVector(Max, Max, Max);
						Center = Elements[i].Bounds.Origin;

						DrawDebugBox(GetWorld(), Center, MaxExtent, FColor().Yellow, false, 0.0f);
						DrawDebugSphere(GetWorld(), Center + MaxExtent, 4.0f, 12, FColor().White, false, 0.0f);
						DrawDebugSphere(GetWorld(), Center - MaxExtent, 4.0f, 12, FColor().White, false, 0.0f);
					}
				});
		}
	}	
}

void UTargetAcquisitionOctreeSubsystem::AddPossibleTargetEntity(const FMassEntityHandle& Entity, int ArmyId)
{
	//Add octrees until armyid, to ensure we don't go out of bounds
	while (ArmyId >= Octrees.Num())
	{
		Octrees.Add(FUnitOctree{Origin, Extent});
		UnitCounts.Add(0);
	}	

	//Calculate bounds
	FMassEntityView View{ *EntityManager, Entity };
	FVector Center = View.GetFragmentData<FTransformFragment>().GetTransform().GetLocation();
	float Radius = View.GetFragmentData<FAgentRadiusFragment>().Radius;
	FBoxSphereBounds Bounds{ Center, FVector{Radius,Radius,Radius}, Radius };

	const FUnitOctreeIdSharedRef& SharedOctreeId = View.GetFragmentData<FUnitOctreeDataFragment>().SharedOctreeId;

	//Insert object in Octree
	Octrees[ArmyId].AddElement(FUnitOctreeElement{ Entity, Bounds, SharedOctreeId });

	++UnitCounts[ArmyId];
}

void UTargetAcquisitionOctreeSubsystem::RemovePossibleTargetEntity(const FMassEntityHandle& Entity, int ArmyId)
{
	FMassEntityView View = FMassEntityView(*EntityManager, Entity);
	
	const FOctreeElementId2& Id = View.GetFragmentData<FUnitOctreeDataFragment>().SharedOctreeId->Id;

	Octrees[ArmyId].RemoveElement(Id);

	--UnitCounts[ArmyId];
}

void UTargetAcquisitionOctreeSubsystem::UpdatePossibleTargetEntity(const FMassEntityHandle& Entity, int ArmyId)
{
	//Calculate bounds
	FMassEntityView View{ *EntityManager, Entity };
	FVector Center = View.GetFragmentData<FTransformFragment>().GetTransform().GetLocation();
	float Radius = View.GetFragmentData<FAgentRadiusFragment>().Radius;
	FBoxSphereBounds Bounds{ Center, FVector{Radius,Radius,Radius}, Radius };

	const FOctreeElementId2& Id = View.GetFragmentData<FUnitOctreeDataFragment>().SharedOctreeId->Id;
	if(!Octrees[ArmyId].IsValidElementId(Id)) return;

	//Make copy
	FUnitOctreeElement ElementCopy = Octrees[ArmyId].GetElementById(Id);
	//Remove element
	Octrees[ArmyId].RemoveElement(Id);
	//Update bounds
	ElementCopy.Bounds = Bounds;
	//Insert element in Octree
	Octrees[ArmyId].AddElement(ElementCopy);
}

const TArray<FUnitOctree>& UTargetAcquisitionOctreeSubsystem::GetOctrees() const
{
	return Octrees;
}