#include "Characters/VSlicesCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "Characters/Components/LandingComponent.h"
#include "Characters/Components/GrapplingHookComponent.h"
#include "Characters/Components/SprintComponent.h"
#include "Characters/Components/SlideComponent.h"
#include "Characters/Components/SlopeComponent.h"
#include "Characters/Components/VaultComponent.h"
#include "Characters/Components/WallRunComponent.h"
#include "CableComponent.h" 
#include "Characters/Components/LedgeSwingComponent.h"

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
	
	// Create movement components
	SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("Sprint"));
	SlideComponent = CreateDefaultSubobject<USlideComponent>(TEXT("Slide"));
	SlopeComponent = CreateDefaultSubobject<USlopeComponent>(TEXT("Slope"));
	LandingComponent = CreateDefaultSubobject<ULandingComponent>(TEXT("Landing"));
	VaultComponent = CreateDefaultSubobject<UVaultComponent>(TEXT("Vault"));
	WallRunComponent = CreateDefaultSubobject<UWallRunComponent>(TEXT("WallRun"));
	GrapplingHookComponent = CreateDefaultSubobject<UGrapplingHookComponent>(TEXT("GrapplingHook"));
	LedgeSwingComponent = CreateDefaultSubobject<ULedgeSwingComponent>(TEXT("LedgeSwing"));
	
	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("GrappleCable"));
	Cable->SetupAttachment(GetMesh(), TEXT("hand_r"));
	Cable->SetVisibility(false);
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
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller && MovementVector.SizeSquared() > 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
		if (SlopeComponent)
		{
			SlopeComponent->ApplySlopeRestrictions(MovementVector);
		}
        
		if (!bIsCrouched && !GetIsSliding() && SprintComponent)
		{
			SprintComponent->SprintCheck(MovementVector.Y, MovementVector.X);
		}
        
		// Apply movement input
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AVSlicesCharacter::Look(const FInputActionValue& Value)
{
	if(VaultComponent && VaultComponent->IsVaulting()) return;
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

#pragma region COMPONENTS

void AVSlicesCharacter::StartSlide() const
{
	if(SlideComponent) SlideComponent->StartSlide();
}

void AVSlicesCharacter::StopSlide() const
{
	if(SlideComponent) SlideComponent->StopSlide();
}

void AVSlicesCharacter::StartSprinting() const
{
	if(SprintComponent) SprintComponent->StartSprinting();
}

void AVSlicesCharacter::StopSprinting() const
{
	if(SprintComponent) SprintComponent->StopSprinting();
}

void AVSlicesCharacter::ShootGrapplingHook() const
{
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
	if (SprintComponent && GetIsSprinting())
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
	if (WallRunComponent->IsWallRunning()) //jump off wall run
	{
		WallRunComponent->Jump();
		GetWorldTimerManager().SetTimer(JumpCooldownTimerHandle,this,&AVSlicesCharacter::ResetJumpCooldown, JumpCooldownTime,false);
		return;
	}
	
	if (LedgeSwingComponent && LedgeSwingComponent->IsHanging())
	{
		LedgeSwingComponent->Jump();
		return;
	}
	if (VaultComponent && !VaultComponent->IsVaulting() && VaultComponent->TryVault(GetIsSprinting()))return; //check vaulting
	if (!bCanJump) return;
    
	Super::Jump(); 
    
	/* if (LedgeSwingComponent)
	// {
	// 	GetWorld()->GetTimerManager().SetTimer(LedgeDetectionTimerHandle, [this]()
	// 	{
	// 		if (!GetCharacterMovement()->IsFalling() || LedgeSwingComponent->IsHanging())
	// 		{
	// 			GetWorld()->GetTimerManager().ClearTimer(LedgeDetectionTimerHandle);
	// 			return;
	// 		}
	// 		LedgeSwingComponent->TryGrab();
	// 	}, 0.01f, true); 
	// } */
    
	if (GetIsSprinting() && GetVelocity().Length()>=MaxJogSpeed) //boost if sprinting
	{
		FTimerHandle JumpTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(JumpTimerHandle, this, &AVSlicesCharacter::LaunchForward, 0.1f, false);
	}
	if(bIsCrouched)
		UnCrouch();
	bCanJump = false;
    
	float CurrentJumpCooldown = JumpCooldownTime;
	if(GetIsSprinting()) CurrentJumpCooldown *= 1.5f;
	GetWorldTimerManager().SetTimer(JumpCooldownTimerHandle,this,&AVSlicesCharacter::ResetJumpCooldown, CurrentJumpCooldown,false);
}

void AVSlicesCharacter::ResetJumpCooldown()
{
	bCanJump = true;
}

void AVSlicesCharacter::LaunchForward()
{
	const float CurrentSpeed = GetVelocity().Length();
	const float SpeedRatio = FMath::Clamp(CurrentSpeed / GetMaxSprintSpeed(), 0.0f, 2.0f); // Cap at 200%
    
	const float SpeedMultiplier = 1.0f + (SpeedRatio * 0.5f); // 1.0 to 2.0 multiplier
    
	const FVector LaunchDir = GetActorForwardVector() * LaunchBoost * SpeedMultiplier;
	LaunchCharacter(LaunchDir, true, false);
}

#pragma endregion JUMP

#pragma region OVERRIDES

void AVSlicesCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other,UPrimitiveComponent* OtherComp, bool bSelfMoved,
	FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if(!OtherComp->IsSimulatingPhysics() && GrapplingHookComponent && GrapplingHookComponent->GetIsGrappling())
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
	if ((PrevMovementMode == MOVE_Walking || PrevMovementMode == MOVE_Flying) && CurrentMovementMode == MOVE_Falling && GetVelocity().Z<=0.f)
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
	
	if (WallRunComponent && WallRunComponent->ShouldCameraTilt() )
		OutResult.Rotation.Roll += WallRunComponent->GetCameraTilt();
}

void AVSlicesCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	WallRunComponent->ResetWallRun();
	WallRunComponent->StopWallRun();
}

#pragma endregion OVERRIDES

#pragma region GETTERS
USprintComponent* AVSlicesCharacter::GetSprintComponent() const
{
	return SprintComponent;
}

USlideComponent* AVSlicesCharacter::GetSlideComponent() const
{
	return SlideComponent;
}

USlopeComponent* AVSlicesCharacter::GetSlopeComponent() const
{
	return SlopeComponent;
}

UVaultComponent* AVSlicesCharacter::GetVaultComponent() const
{
	return VaultComponent;
}

ULandingComponent* AVSlicesCharacter::GetLandingComponent() const
{
	return LandingComponent;
}

UWallRunComponent* AVSlicesCharacter::GetWallRunComponent() const
{
	return WallRunComponent;
}

FSlopeInfo AVSlicesCharacter::GetSlopeInfo() const
{
	return SlopeComponent->GetSlopeInfo();
}

bool AVSlicesCharacter::GetIsSprinting() const
{
	return SprintComponent->GetIsSprinting();
}

bool AVSlicesCharacter::GetIsSliding() const
{
	return SlideComponent ? SlideComponent->IsSliding() : false;
}
#pragma endregion GETTERS