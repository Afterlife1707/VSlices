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
	
    bool TryGrab();
	void OnJump();

protected:
	virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float OriginalGravityScale;
};