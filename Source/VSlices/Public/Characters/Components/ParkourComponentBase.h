#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LoggingMacros.h" //for child classes
#include "GameFramework/CharacterMovementComponent.h"
#include "ParkourComponentBase.generated.h"

UCLASS(Abstract, ClassGroup=(Parkour))
class VSLICES_API UParkourComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UParkourComponentBase();

protected:
	virtual void OnComponentCreated() override;
	virtual void BeginPlay() override;
	
	UPROPERTY()
	class AVSlicesCharacter* OwnerCharacter;
	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;
};