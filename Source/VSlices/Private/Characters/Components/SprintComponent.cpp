#include "Characters/Components/SprintComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

USprintComponent::USprintComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USprintComponent::BeginPlay()
{
    Super::BeginPlay();

    if (MovementComponent)
    {
        MovementComponent->MaxWalkSpeed = OwnerCharacter->GetMaxJogSpeed();
        MovementComponent->MaxWalkSpeedCrouched = OwnerCharacter->GetMaxCrouchJogSpeed();
    }
    else
    {
        LOG_ERROR("Invalid Movement Component");
    }

    CurrentStamina = MaxStamina;

    if (BreathingSound)
    {
        BreathingAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), BreathingSound, 1.f, 1.f, 0.f, nullptr, true, false);
        if (BreathingAudioComponent)
            BreathingAudioComponent->Stop();
    }
    else
        WarnMissingAsset(TEXT("BreathingSound"));
}

void USprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateStamina(DeltaTime);
    if (bIsSprinting || bExhausted || CurrentStamina < StaminaBreathingThreshold)
        UpdateBreathingAudio();
}

void USprintComponent::UpdateStamina(const float DeltaTime)
{
    const bool bIsActuallyMoving = OwnerCharacter->GetVelocity().SizeSquared() > OwnerCharacter->GetMaxJogSpeed() * OwnerCharacter->GetMaxJogSpeed();
    if (bIsSprinting && bIsActuallyMoving)
    {
        CurrentStamina = FMath::Clamp(CurrentStamina - StaminaDrainRate * DeltaTime, 0.f, MaxStamina);
        RegenDelayRemaining = StaminaRegenDelay;
        if (CurrentStamina <= 0.f && !bExhausted)
        {
            bExhausted = true;
            StopSprinting();
        }
    }
    else
    {
        if (CurrentStamina >= MaxStamina && !bExhausted) return;
        
        if (RegenDelayRemaining > 0.f)
        {
            RegenDelayRemaining -= DeltaTime;
            return;
        }
        CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRegenRate * DeltaTime, 0.f, MaxStamina);
        if (bExhausted && CurrentStamina >= StaminaRegenThreshold)
            bExhausted = false;
    }
}

void USprintComponent::UpdateBreathingAudio() const
{
    if (!BreathingAudioComponent) return;
    if (const bool bShouldBreathe = (bIsSprinting || bExhausted) && CurrentStamina < StaminaBreathingThreshold; !bShouldBreathe)
    {
        if (BreathingAudioComponent->IsPlaying())
        {
            BreathingAudioComponent->AdjustVolume(2.f, 0.f); // Fade out
        }
        return;
    }

    if (!BreathingAudioComponent->IsPlaying())
        BreathingAudioComponent->Play();

    // Normalise stamina within breathing range (0 = exhausted, 1 = just started breathing)
    const float StaminaRatio = FMath::Clamp(CurrentStamina / StaminaBreathingThreshold, 0.f, 1.f);

    // Higher exhaustion = louder and higher pitch // or change the breathing sounds here
    const float TargetVolume = FMath::Lerp(1.f, 0.7f, StaminaRatio);
    const float TargetPitch  = FMath::Lerp(1.3f, 0.9f, StaminaRatio);

    BreathingAudioComponent->SetVolumeMultiplier(TargetVolume);
    BreathingAudioComponent->SetPitchMultiplier(TargetPitch);
}

void USprintComponent::StartSprinting()
{
    if (!bCanSprint || bSprintOnCooldown || bExhausted || !OwnerCharacter || !MovementComponent)
        return;

    bIsSprinting = true;
    if (!OwnerCharacter->bIsCrouched)
        MovementComponent->MaxWalkSpeed = OwnerCharacter->GetMaxSprintSpeed();
    else
        MovementComponent->MaxWalkSpeedCrouched = OwnerCharacter->GetMaxCrouchSprintSpeed();
}

void USprintComponent::StopSprinting()
{
    if (!OwnerCharacter || !MovementComponent) return;
    bIsSprinting = false;
    if (!OwnerCharacter->bIsCrouched)
        MovementComponent->MaxWalkSpeed = OwnerCharacter->GetMaxJogSpeed();
    else
        MovementComponent->MaxWalkSpeedCrouched = OwnerCharacter->GetMaxCrouchJogSpeed();
}

void USprintComponent::SprintCheck(const float ForwardValue, const float RightValue)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        LOG_ERROR("SprintComponent: Missing OwnerCharacter or MovementComponent in SprintCheck");
        return;
    }

    const bool bMovingForward  = ForwardValue > 0.001f;
    const bool bMovingBackward = ForwardValue < -0.001f;
    const bool bMovingSideways = FMath::Abs(RightValue) > 0.001f;
    
    bCanSprint = !bMovingSideways && !bMovingBackward && bMovingForward && !bSprintOnCooldown && !bExhausted && CurrentStamina > 0.f;

    if (bIsSprinting && (!bCanSprint || bSprintOnCooldown))
    {
        StopSprinting();
        StartSprintCooldown();
    }
}

void USprintComponent::StartSprintCooldown()
{
    if (SprintCooldownDuration > 0.0f)
    {
        bSprintOnCooldown = true;
        GetWorld()->GetTimerManager().SetTimer(SprintCooldownTimerHandle, this, &USprintComponent::EndSprintCooldown, SprintCooldownDuration, false);
    }
}

void USprintComponent::EndSprintCooldown()
{
    bSprintOnCooldown = false;
    GetWorld()->GetTimerManager().ClearTimer(SprintCooldownTimerHandle);
}