#pragma once

#include "CoreMinimal.h"
#include "Characters/Components/ParkourComponentBase.h"
#include "LedgeSwingComponent.generated.h"

UENUM(BlueprintType)
enum class EHangType : uint8
{
	None,
	Ledge,      // Flat surface - can mantle up
	Pole        // Cylindrical - can swing
};

UCLASS()
class VSLICES_API ULedgeSwingComponent : public UParkourComponentBase
{
	GENERATED_BODY()
public:
    ULedgeSwingComponent();
	
    bool TryGrab();
    void Jump();
    void Drop(); //need an input for this
    
    bool IsHanging() const { return bIsHanging; }
    EHangType GetCurrentHangType() const { return CurrentHangType; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool bIsHanging = false;
    EHangType CurrentHangType = EHangType::None;
    FVector HangLocation;
    FVector HangNormal;
    float SwingAngle;
    float SwingVelocity;
    float InitialMomentum;
    
    // Detection parameters
    UPROPERTY(EditAnywhere, Category = "Detection", meta=(AllowPrivateAccess))
    float ForwardReachDistance = 150.0f;
    UPROPERTY(EditAnywhere, Category = "Detection", meta=(AllowPrivateAccess))
    float UpwardReachDistance = 120.0f;
    UPROPERTY(EditAnywhere, Category = "Detection", meta=(AllowPrivateAccess))
    float DownwardSearchDistance = 150.0f;
    UPROPERTY(EditAnywhere, Category = "Detection", meta=(AllowPrivateAccess))
    float MinGrabHeight = 50.0f; // Must be this high off ground
	
    // Swing parameters (poles)
    UPROPERTY(EditAnywhere, Category = "Swing", meta=(AllowPrivateAccess))
    float MaxSwingAngle = 45.0f;
    UPROPERTY(EditAnywhere, Category = "Swing", meta=(AllowPrivateAccess))
    float SwingForce = 500.0f;
    UPROPERTY(EditAnywhere, Category = "Swing", meta=(AllowPrivateAccess))
    float SwingDecay = 0.95f;
    UPROPERTY(EditAnywhere, Category = "Swing", meta=(AllowPrivateAccess))
    float SwingJumpMultiplier = 800.0f;
	
    // Mantle parameters (ledges)
    UPROPERTY(EditAnywhere, Category = "Mantle", meta=(AllowPrivateAccess))
    float MantleHeight = 200.0f;
    UPROPERTY(EditAnywhere, Category = "Mantle", meta=(AllowPrivateAccess))
    float MantleForwardDistance = 50.0f;
    
    // Functions
    void StartHang(const FVector& Location, const FVector& Normal, EHangType HangType);
    void UpdateSwing(float DeltaTime);
    void UpdateHangPosition();
    void ReleaseHang();
    
    bool DetectLedge(FVector& OutLocation, FVector& OutNormal);
    bool DetectPole(FVector& OutLocation, FVector& OutNormal);
    
    EHangType DetermineHangType(const FVector& Normal) const;
    
    void SwingJump();
    void MantleUp();
    
    float CalculateSwingMomentum() const;
	void DebugDrawGrabAttempt();
};