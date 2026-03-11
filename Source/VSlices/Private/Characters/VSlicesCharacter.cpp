#include "Characters/VSlicesCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "CableComponent.h"
#include "Characters/Components/SprintComponent.h"
#include "Characters/Components/SlideComponent.h"
#include "Characters/Components/SlopeComponent.h"
#include "Characters/Components/VaultComponent.h"
#include "Characters/Components/LandingComponent.h"
#include "Characters/Components/WallRunComponent.h"
#include "Characters/Components/GrapplingHookComponent.h"
#include "Characters/Components/LedgeComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AVSlicesCharacter

AVSlicesCharacter::AVSlicesCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), TEXT("neck_02"));
	CameraBoom->TargetArmLength = 0.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("GrappleCable"));
	Cable->SetupAttachment(GetMesh(), TEXT("hand_r"));
	Cable->SetVisibility(false);

	// Parkour components are added freely via Blueprint
}

void AVSlicesCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Cache whatever parkour components have been added in Blueprint
	SprintComponent        = FindComponentByClass<USprintComponent>();
	SlideComponent         = FindComponentByClass<USlideComponent>();
	SlopeComponent         = FindComponentByClass<USlopeComponent>();
	LandingComponent       = FindComponentByClass<ULandingComponent>();
	VaultComponent         = FindComponentByClass<UVaultComponent>();
	WallRunComponent       = FindComponentByClass<UWallRunComponent>();
	GrapplingHookComponent = FindComponentByClass<UGrapplingHookComponent>();
	LedgeComponent         = FindComponentByClass<ULedgeComponent>();
}

void AVSlicesCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bInCoyoteTime)
	{
		CoyoteTimeRemaining -= DeltaSeconds;
		if (CoyoteTimeRemaining <= 0.0f)
		{
			bInCoyoteTime = false;
			CoyoteTimeRemaining = 0.0f;
		}
	}

	if (SlideComponent)
		SlideComponent->HandleSlideTick(DeltaSeconds);

	if (LandingComponent)
		LandingComponent->HandleFallDetection();
}

void AVSlicesCharacter::Move(const FInputActionValue& Value)
{
	if (bLedgeGrab) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (!Controller || MovementVector.SizeSquared() <= 0.0f) return;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (SlopeComponent)
		SlopeComponent->ApplySlopeRestrictions(MovementVector);

	if (!bIsCrouched && !GetIsSliding() && SprintComponent)
		SprintComponent->SprintCheck(MovementVector.Y, MovementVector.X);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AVSlicesCharacter::Look(const FInputActionValue& Value)
{
	if (VaultComponent && VaultComponent->IsVaulting()) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller == nullptr) return;

	if (bLedgeGrab)
	{
		FRotator ControlRotation = Controller->GetControlRotation();
		FRotator ActorRotation = GetActorRotation();
		const float RelativeYaw = FRotator::NormalizeAxis(ControlRotation.Yaw - ActorRotation.Yaw);
		const float NewRelativeYaw = FMath::Clamp(RelativeYaw + LookAxisVector.X, -60.0f, 60.0f);

		ControlRotation.Yaw = ActorRotation.Yaw + NewRelativeYaw;
		Controller->SetControlRotation(ControlRotation);
	}
	else
	{
		AddControllerYawInput(LookAxisVector.X);
	}

	AddControllerPitchInput(LookAxisVector.Y);
}

#pragma region COMPONENTS

void AVSlicesCharacter::StartSlide() const
{
	if (!SlideComponent) { WarnMissingComponent(USlideComponent::StaticClass()); return; }
	SlideComponent->StartSlide();
}

void AVSlicesCharacter::StopSlide() const
{
	if (!SlideComponent) { WarnMissingComponent(USlideComponent::StaticClass()); return; }
	SlideComponent->StopSlide();
}

void AVSlicesCharacter::StartSprinting() const
{
	if (!SprintComponent) { WarnMissingComponent(USprintComponent::StaticClass()); return; }
	SprintComponent->StartSprinting();
}

void AVSlicesCharacter::StopSprinting() const
{
	if (!SprintComponent) { WarnMissingComponent(USprintComponent::StaticClass()); return; }
	SprintComponent->StopSprinting();
}

void AVSlicesCharacter::ShootGrapplingHook() const
{
	if (!GrapplingHookComponent) { WarnMissingComponent(UGrapplingHookComponent::StaticClass()); return; }
	GrapplingHookComponent->TryShoot();
}

void AVSlicesCharacter::ToggleCrouch()
{
	if (bIsCrouched) StopCrouch();
	else StartCrouch();
}

void AVSlicesCharacter::StartCrouch()
{
	Crouch();
	if (GetIsSprinting())
		StartSlide();
}

void AVSlicesCharacter::StopCrouch()
{
	UnCrouch();
}

#pragma endregion COMPONENTS

#pragma region JUMP

void AVSlicesCharacter::Jump()
{
	if (LedgeComponent && bLedgeGrab)
	{
		LedgeComponent->OnJump();
		return;
	}

	if (WallRunComponent && WallRunComponent->IsWallRunning())
	{
		WallRunComponent->Jump();
		GetWorldTimerManager().SetTimer(JumpCooldownTimerHandle, this, &AVSlicesCharacter::ResetJumpCooldown, JumpCooldownTime, false);
		return;
	}

	if (VaultComponent && !VaultComponent->IsVaulting() && VaultComponent->TryVault(GetIsSprinting()))
		return;

	if (!bCanJump) return;

	Super::Jump();

	if (GetIsSprinting() && GetVelocity().Length() >= MaxJogSpeed)
	{
		FTimerHandle JumpTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(JumpTimerHandle, this, &AVSlicesCharacter::LaunchForward, 0.1f, false);
	}

	if (bIsCrouched)
		UnCrouch();

	bCanJump = false;

	float CurrentJumpCooldown = JumpCooldownTime;
	if (GetIsSprinting()) CurrentJumpCooldown *= 1.5f;

	GetWorldTimerManager().SetTimer(JumpCooldownTimerHandle, this, &AVSlicesCharacter::ResetJumpCooldown, CurrentJumpCooldown, false);
}

void AVSlicesCharacter::ResetJumpCooldown()
{
	bCanJump = true;
}

void AVSlicesCharacter::LaunchForward()
{
	const float CurrentSpeed = GetVelocity().Length();
	const float SpeedRatio = FMath::Clamp(CurrentSpeed / GetMaxSprintSpeed(), 0.0f, 2.0f);
	const float SpeedMultiplier = 1.0f + (SpeedRatio * 0.5f);

	const FVector LaunchDir = GetActorForwardVector() * LaunchBoost * SpeedMultiplier;
	LaunchCharacter(LaunchDir, true, false);
}

#pragma endregion JUMP

#pragma region OVERRIDES

void AVSlicesCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other,
	UPrimitiveComponent* OtherComp, bool bSelfMoved,
	FVector HitLocation, FVector HitNormal,
	FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (OtherComp && !OtherComp->IsSimulatingPhysics() && GrapplingHookComponent && GrapplingHookComponent->GetIsGrappling())
		GrapplingHookComponent->ClimbAtEnd();

	if (WallRunComponent && !WallRunComponent->IsWallRunning() && Other && OtherComp)
		WallRunComponent->TryWallRun(Hit);
}

bool AVSlicesCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() || bIsCrouched || bInCoyoteTime;
}

void AVSlicesCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	const EMovementMode CurrentMovementMode = GetCharacterMovement()->MovementMode;

	if ((PrevMovementMode == MOVE_Walking || PrevMovementMode == MOVE_Flying) && CurrentMovementMode == MOVE_Falling && GetVelocity().Z <= 0.f)
	{
		bInCoyoteTime = true;
		CoyoteTimeRemaining = CoyoteTimeDuration;
	}

	if (CurrentMovementMode == MOVE_Walking || CurrentMovementMode == MOVE_Flying)
	{
		bInCoyoteTime = false;
		CoyoteTimeRemaining = 0.0f;
	}
}

void AVSlicesCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);

	if (WallRunComponent && WallRunComponent->ShouldCameraTilt())
		OutResult.Rotation.Roll += WallRunComponent->GetCameraTilt();
}

void AVSlicesCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (WallRunComponent)
	{
		WallRunComponent->ResetWallRun();
		WallRunComponent->StopWallRun();
	}
}

#pragma endregion OVERRIDES

#pragma region GETTERS

FSlopeInfo AVSlicesCharacter::GetSlopeInfo() const
{
	check(SlopeComponent);
	return SlopeComponent->GetSlopeInfo();
}

bool AVSlicesCharacter::GetIsSprinting() const
{
	return SprintComponent ? SprintComponent->GetIsSprinting() : false;
}

bool AVSlicesCharacter::GetIsSliding() const
{
	return SlideComponent ? SlideComponent->IsSliding() : false;
}

void AVSlicesCharacter::WarnMissingComponent(const TSubclassOf<UActorComponent> ComponentClass) const
{
	if (const FString ClassName = ComponentClass ? ComponentClass->GetName() : TEXT("Unknown Component"); !AlreadyWarned.Contains(ClassName))
	{
		AlreadyWarned.Add(ClassName);
		LOG_WARNING("%s: %s is missing. Add it in the Blueprint Components panel.", *GetName(), *ClassName);
	}
}

#pragma endregion GETTERS