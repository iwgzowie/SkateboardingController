// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/SkateControllerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <Kismet/KismetMathLibrary.h>
#include "UI/ScoreWidget.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);


ASkateControllerCharacter::ASkateControllerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.0f;	
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SkateboardMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkateboardMesh"));
	SkateboardMesh->SetupAttachment(RootComponent, NAME_None);

}

void ASkateControllerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Crear y agregar el Widget de Score a la pantalla
	if (ScoreWidgetClass)
	{
		ScoreWidget = CreateWidget<UScoreWidget>(GetWorld(), ScoreWidgetClass);
		if (ScoreWidget)
		{
			ScoreWidget->AddToViewport();
			ScoreWidget->UpdateScore(Score);
		}
	}
}


void ASkateControllerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AlignWithGround();

}

FVector ASkateControllerCharacter::PerformWheelTrace(const FVector& StartLocation)
{
	FVector TraceStart = StartLocation + FVector(0, 0, 30);
	FVector TraceEnd = StartLocation - FVector(0, 0, 30);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return HitResult.ImpactPoint;
	}

	return StartLocation;
}

void ASkateControllerCharacter::AlignWithGround()
{
	if (!SkateboardMesh) return;

	ASkateControllerCharacter* SkateChar = Cast<ASkateControllerCharacter>(GetOwner());
	if (SkateChar && SkateChar->GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// Obtener posiciones de los sockets
	FVector FrontSocketLocation = SkateboardMesh->GetSocketLocation("S_Front");
	FVector BackSocketLocation = SkateboardMesh->GetSocketLocation("S_Back");

	// Obtener puntos de impacto en el terreno
	FVector FrontHitLocation = PerformWheelTrace(FrontSocketLocation);
	FVector BackHitLocation = PerformWheelTrace(BackSocketLocation);

	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(BackHitLocation, FrontHitLocation);

	// Calcular diferencia angular
	FRotator DeltaRotation = (TargetRotation - SkateboardMesh->GetComponentRotation()).GetNormalized();

	// Aplicar solo si supera el umbral de tolerancia
	if (FMath::Abs(DeltaRotation.Pitch) > 5.0f || FMath::Abs(DeltaRotation.Roll) > 5.0f)
	{
		SkateboardMesh->SetWorldRotation(TargetRotation);
	}

}

void ASkateControllerCharacter::AddScore(int32 Points)
{
	Score += Points;

	if (ScoreWidget)
	{
		ScoreWidget->UpdateScore(Score);
	}
}



//////////////////////////////////////////////////////////////////////////
// Input

void ASkateControllerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASkateControllerCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASkateControllerCharacter::ReleaseJump);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateControllerCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateControllerCharacter::Look);


		EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Started, this, &ASkateControllerCharacter::StartPush);
		EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Completed, this, &ASkateControllerCharacter::StopPush);

		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ASkateControllerCharacter::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ASkateControllerCharacter::StopBrake);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASkateControllerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASkateControllerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASkateControllerCharacter::StartPush()
{
	bIsPushing = true;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * PushMultiplier;
}

void ASkateControllerCharacter::StopPush()
{
	bIsPushing = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
}

void ASkateControllerCharacter::StartBrake()
{
	bIsBraking = true;
	GetCharacterMovement()->BrakingDecelerationWalking = 900.0f;
}

void ASkateControllerCharacter::StopBrake()
{
	bIsBraking = false;
	GetCharacterMovement()->BrakingDecelerationWalking = 0.0f;
}

void ASkateControllerCharacter::Jump()
{
	Super::Jump();
}

void ASkateControllerCharacter::ReleaseJump()
{
	UE_LOG(LogTemp, Warning, TEXT("ReleaseJump()"));

}


