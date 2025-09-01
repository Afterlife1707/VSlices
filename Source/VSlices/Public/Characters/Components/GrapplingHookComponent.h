#pragma once

#include "CoreMinimal.h"
#include "ParkourComponentBase.h"
#include "GrapplingHookComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VSLICES_API UGrapplingHookComponent : public UParkourComponentBase
{
    GENERATED_BODY()

public: 
    UGrapplingHookComponent();
    void TryShoot();
    void ReleaseGrapple();
    void ClimbAtEnd();
    FORCEINLINE bool GetIsGrappling() const { return bIsGrappling; }
    FORCEINLINE bool ShouldBoost() const { return bShouldBoost; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Grappling Parameters
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grappling", meta = (AllowPrivateAccess = "true"))
    float Range = 3000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling", meta = (AllowPrivateAccess = "true"))
    float GrappleCooldown = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling", meta = (AllowPrivateAccess = "true"))
    float ClimbDistance = 800.0f;

    // Force Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces", meta = (AllowPrivateAccess = "true"))
    float InitialUpwardBoost = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces", meta = (AllowPrivateAccess = "true"))
    float BasePullStrength = 2000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces", meta = (AllowPrivateAccess = "true"))
    float DownwardPullMultiplier = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forces", meta = (AllowPrivateAccess = "true"))
    float HorizontalAntiGravityForce = 800.0f;

    // Distance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distances", meta = (AllowPrivateAccess = "true"))
    float ReleaseDistance = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distances", meta = (AllowPrivateAccess = "true"))
    float ClimbTriggerDistance = 200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distances", meta = (AllowPrivateAccess = "true"))
    float DistanceScaleReference = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distances", meta = (AllowPrivateAccess = "true"))
    float HorizontalGrappleThreshold = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distances", meta = (AllowPrivateAccess = "true"))
    float MinHorizontalDistanceForAntiGravity = 200.0f;

    // Boost Conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (AllowPrivateAccess = "true"))
    float MinVerticalBoostHeight = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (AllowPrivateAccess = "true"))
    float MinHorizontalBoostDistance = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (AllowPrivateAccess = "true"))
    float CableInterpSpeed = 30.0f;

    float CurrentCooldown;
    bool bIsGrappling;
    bool bShouldBoost;
    FVector GrappleLocation;
    float Distance;
    float OriginalCapsuleHalfHeight;
    
    // Force Scaling Constants
    static constexpr float MinDistanceMultiplier = 0.5f;
    static constexpr float MaxDistanceMultiplier = 2.0f;

    // Private Methods
    void StartGrapple(const FVector& TargetLocation);
    void UpdateCableVisuals(float DeltaTime) const;
    float CalculatePullStrength(const FVector& ToTarget) const;
    bool ShouldApplyAntiGravity(const FVector& ToTarget) const;
};