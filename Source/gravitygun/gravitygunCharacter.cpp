// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "gravitygunCharacter.h"
#include "gravitygunProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AgravitygunCharacter

AgravitygunCharacter::AgravitygunCharacter()
{

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);


	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	HeldObjectLocation = FP_MuzzleLocation;
	isHoldingObject = false;
	EndTracePoint = 1000.f;
	GravGunImpulseAmt = 5000.f;
	//////////////////////////////////////////////////////////////////////////
	//Custom Section End
	//////////////////////////////////////////////////////////////////////////


}

void AgravitygunCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	
}

void AgravitygunCharacter::Tick(float DeltaTime)
{
	PhysicsHandle->SetTargetLocation(FP_MuzzleLocation->GetComponentLocation());
	PhysicsHandle->SetTargetRotation(FP_MuzzleLocation->GetComponentRotation());
}

//////////////////////////////////////////////////////////////////////////
// Input

void AgravitygunCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("GrabObject", IE_Pressed, this, &AgravitygunCharacter::GrabObject);
	PlayerInputComponent->BindAction("ReleaseObject", IE_Pressed, this, &AgravitygunCharacter::ReleaseObject);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AgravitygunCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AgravitygunCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AgravitygunCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AgravitygunCharacter::LookUpAtRate);
}

//void AgravitygunCharacter::OnFire()
//{
//	// try and fire a projectile
//	if (ProjectileClass != NULL)
//	{
//		UWorld* const World = GetWorld();
//		if (World != NULL)
//		{
//				const FRotator SpawnRotation = GetControlRotation();
//				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
//				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
//
//				//Set Spawn Collision Handling Override
//				FActorSpawnParameters ActorSpawnParams;
//				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
//
//				// spawn the projectile at the muzzle
//				World->SpawnActor<AgravitygunProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
//			
//		}
//	}
//
//	// try and play the sound if specified
//	if (FireSound != NULL)
//	{
//		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
//	}
//
//	// try and play a firing animation if specified
//	if (FireAnimation != NULL)
//	{
//		// Get the animation object for the arms mesh
//		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
//		if (AnimInstance != NULL)
//		{
//			AnimInstance->Montage_Play(FireAnimation, 1.f);
//		}
//	}
//}

void AgravitygunCharacter::GrabObject()
{
	if(!isHoldingObject)
	{
		FVector StartTrace = FP_Gun->GetComponentLocation();
		FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
		FVector EndTrace = ((ForwardVector * EndTracePoint) + StartTrace);
		FCollisionQueryParams CollisionParams;

		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, true);

		bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, StartTrace, EndTrace, ECC_Visibility, CollisionParams);

		if (isHit && isHoldingObject == false)
		{
			if (OutHit.bBlockingHit)
			{
				if (OutHit.GetComponent()->IsSimulatingPhysics())
				{
					PhysicsHandle->GrabComponentAtLocationWithRotation(OutHit.GetComponent(), "None", OutHit.GetComponent()->GetComponentLocation(), FP_MuzzleLocation->GetComponentRotation());
					isHoldingObject = true;
				}
			}
		}
	}

	else if (isHoldingObject)
	{
		LaunchObject();
	}

}

void AgravitygunCharacter::LaunchObject()
{
	PhysicsHandle->ReleaseComponent();
	OutHit.GetComponent()->AddImpulse((FirstPersonCameraComponent->GetForwardVector() * GravGunImpulseAmt), "None", true);
	isHoldingObject = false;
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Fire!");
}

void AgravitygunCharacter::ReleaseObject()
{
	if (isHoldingObject)
	{
		PhysicsHandle->ReleaseComponent();
		isHoldingObject = false;
	}
}

void AgravitygunCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		GrabObject();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AgravitygunCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AgravitygunCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AgravitygunCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AgravitygunCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AgravitygunCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AgravitygunCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AgravitygunCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AgravitygunCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AgravitygunCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AgravitygunCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}
