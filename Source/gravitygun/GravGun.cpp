// Fill out your copyright notice in the Description page of Project Settings.


#include "GravGun.h"
#include "DrawDebugHelpers.h"

// Sets default values
AGravGun::AGravGun()
{

	//Skeletal Mesh
	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	RootComponent = GunMesh;

	HeldObjectLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Held Object Location"));


	
	//Where object moves to once it's registered as 'picked up'
	//FVector HeldObjectLocation;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGravGun::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGravGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGravGun::PickupObject()
{
	FHitResult OutHit;
	FVector StartTrace = GunMesh->GetComponentLocation();
	FVector ForwardVector = GetOwner()->GetActorForwardVector();
	FVector EndTrace = ((ForwardVector * 1000.f) + StartTrace);
	FCollisionQueryParams CollisionParams;

	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, true);

	bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartTrace,EndTrace, ECC_Visibility, CollisionParams);

	if (isHit)
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.GetComponent()->IsSimulatingPhysics())
			{
				PhysicsHandle->GrabComponentAtLocation(OutHit.GetComponent(), "None",HeldObjectLocation->GetComponentLocation());
				isHoldingObject = true;
			}
		}
	}
}

