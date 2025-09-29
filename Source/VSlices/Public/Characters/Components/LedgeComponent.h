#pragma once

#include "CoreMinimal.h"
#include "Characters/Components/ParkourComponentBase.h"
#include "LedgeComponent.generated.h"

UCLASS()
class VSLICES_API ULedgeComponent : public UParkourComponentBase
{
	GENERATED_BODY()
public:
    ULedgeComponent();
	
    void TryGrab() const;
	void OnJump() const;
	void SetGrab(bool bGrab) const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Ledge|Detection")
	float ForwardTraceDistance = 50.f;
	UPROPERTY(EditAnywhere, Category = "Ledge|Detection")
	float DownwardTraceDistance = 50.f;
	UPROPERTY(EditAnywhere, Category = "Ledge|Detection")
	float LedgeHeightOffset = 10.f;
	UPROPERTY(EditAnywhere, Category = "Ledge|Positioning")
	float WallOffsetDistance = -25.f;
	UPROPERTY(EditAnywhere, Category = "Ledge|Positioning")
	float HangHeightMultiplier = 1.5f;
	UPROPERTY(EditAnywhere, Category = "Ledge|Jump")
	float JumpVelocityMultiplier = 1.f;

private:
	float OriginalGravityScale;
	float CapsuleHalfHeight;
	float CapsuleRadius;
};