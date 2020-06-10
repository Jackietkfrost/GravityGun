// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravGun.generated.h"


UCLASS()
class GRAVITYGUN_API AGravGun : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGravGun();

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* GunMesh;

	//Where object moves to once it's registered as 'picked up'
	UPROPERTY(EditAnywhere)
	USceneComponent* HeldObjectLocation;

	UPROPERTY()
	UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY()
	bool isHoldingObject;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void PickupObject();

};
